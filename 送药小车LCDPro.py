import os, gc, time
from machine import UART, FPIOA, Pin
from libs.PlatTasks import DetectionApp
from libs.PipeLine import PipeLine
from libs.Utils import *

# =========================================================
# 1. UART1 初始化
# =========================================================
fpioa = FPIOA()

# UART1 引脚映射（按你当前配置）
fpioa.set_function(3, fpioa.UART1_TXD)   # Pin 8
fpioa.set_function(4, fpioa.UART1_RXD)   # Pin 10

uart = UART(
    UART.UART1,
    baudrate=115200,
    bits=UART.EIGHTBITS,
    parity=UART.PARITY_NONE,
    stop=UART.STOPBITS_ONE
)

# =========================================================
# 2. 板载按键初始化
# =========================================================
# 你给的按键定义：GPIO53
fpioa.set_function(53, FPIOA.GPIO53)
key = Pin(53, Pin.IN, Pin.PULL_DOWN)

# =========================================================
# 3. 显示/模型参数
# =========================================================
display_mode = "lcd"
rgb888p_size = [1280, 720]
display_size = [800, 480]

root_path = "/sdcard/mark_sonnet_4.7/mp_deployment_source/"

deploy_conf = read_json(root_path + "/deploy_config.json")
kmodel_path = root_path + deploy_conf["kmodel_path"]
labels = deploy_conf["categories"]
confidence_threshold = deploy_conf["confidence_threshold"]
nms_threshold = deploy_conf["nms_threshold"]
model_input_size = deploy_conf["img_size"]
nms_option = deploy_conf["nms_option"]
model_type = deploy_conf["model_type"]

anchors = []
if model_type == "AnchorBaseDet":
    anchors = deploy_conf["anchors"][0] + deploy_conf["anchors"][1] + deploy_conf["anchors"][2]

inference_mode = "video"
debug_mode = 0

# =========================================================
# 4. 创建显示/推理管线
# =========================================================
pl = PipeLine(
    rgb888p_size=rgb888p_size,
    display_size=display_size,
    display_mode=display_mode
)
pl.create()

frame_center_x = display_size[0] / 2   # 用检测框同一坐标系做比较

det_app = DetectionApp(
    inference_mode,
    kmodel_path,
    labels,
    model_input_size,
    anchors,
    model_type,
    confidence_threshold,
    nms_threshold,
    rgb888p_size,
    display_size,
    debug_mode=debug_mode
)

det_app.config_preprocess()

# =========================================================
# 5. 状态变量
# =========================================================
target_digit = None          # 按键拍下后保存的目标数字
last_key_state = 0           # 用于检测按键上升沿
last_uart_debug_time = 0     # 调试信息发送节流
last_dir_send_time = 0       # L/R发送节流
last_sent_dir = ""           # 最近一次实际发送的方向："" / "L" / "R"
target_lock_time = 0         # 锁定目标数字的时间
last_dir_on_lcd = ""         # LCD左上角显示最近一次实际发送的方向

DIR_SEND_INTERVAL = 500      # 两次方向发送最小间隔(ms)
DEBUG_SEND_INTERVAL = 200    # 调试输出间隔(ms)
LOCK_DELAY_MS = 2000         # 锁定目标后等待2秒，再开始判断方向
CENTER_BIAS = 400            #中心点死区补偿
# =========================================================
# 6. 工具函数
# =========================================================
def send_uart(msg):
    try:
        uart.write(msg)
    except Exception as e:
        print("UART发送失败:", e)

def send_uart_repeat(msg, repeat=5, interval_ms=30):
    """
    连续快速发送多次，增强单片机接收成功率
    """
    try:
        for _ in range(repeat):
            uart.write(msg)
            time.sleep_ms(interval_ms)
    except Exception as e:
        print("UART连续发送失败:", e)

def parse_detections(res):
    """
    把检测结果解析成列表，每个元素:
    {
        'digit': '3',
        'score': 0.92,
        'x': ...,
        'y': ...,
        'w': ...,
        'h': ...,
        'center_x': ...
    }
    """
    detections = []

    if not res:
        return detections
    if ('idx' not in res) or ('scores' not in res) or ('boxes' not in res):
        return detections
    if len(res['idx']) == 0:
        return detections

    for i in range(len(res['idx'])):
        try:
            class_id = int(res['idx'][i])
            digit_str = str(labels[class_id])
            score = float(res['scores'][i])

            box = res['boxes'][i]
            x = float(box[0])
            y = float(box[1])
            w = float(box[2])
            h = float(box[3])
            center_x = x + w / 2

            detections.append({
                'digit': digit_str,
                'score': score,
                'x': x,
                'y': y,
                'w': w,
                'h': h,
                'center_x': center_x
            })
        except Exception as e:
            print("解析单个目标失败:", e)

    # 按横坐标从左到右排序
    detections.sort(key=lambda d: d['center_x'])
    return detections

def choose_target_digit(detections):
    """
    按键按下时，从当前帧里选一个数字作为目标数字。
    默认选择置信度最高的那个。
    """
    if not detections:
        return None
    best = max(detections, key=lambda d: d['score'])
    return best['digit']

def find_matching_target(detections, target_digit):
    """
    在当前检测结果中，找与目标数字相同的那个。
    若有多个相同数字，优先选择置信度最高的。
    """
    matched = []
    for d in detections:
        if d['digit'] == str(target_digit):
            matched.append(d)

    if not matched:
        return None

    best_match = max(matched, key=lambda d: d['score'])
    return best_match

# =========================================================
# 7. 主循环
# =========================================================
try:
    while True:
        with ScopedTiming("total", 0):
            img = pl.get_frame()
            res = det_app.run(img)

            detections = parse_detections(res)

            # -------------------------------------------------
            # A. 一直输出当前识别到的数字，方便串口终端调试
            # -------------------------------------------------
            now = time.ticks_ms()
            if time.ticks_diff(now, last_uart_debug_time) > DEBUG_SEND_INTERVAL:
                if len(detections) > 0:
                    digits_str = ",".join([d['digit'] for d in detections])
                    debug_msg = "DETECT:{}\n".format(digits_str)
                else:
                    debug_msg = "DETECT:None\n"

                print(debug_msg.strip())
                ## send_uart(debug_msg)
                last_uart_debug_time = now

            # -------------------------------------------------
            # B. 检测按键上升沿：按下时锁定目标数字
            # -------------------------------------------------
            key_state = key.value()

            # 上升沿：0 -> 1
            if key_state == 1 and last_key_state == 0:
                print("按键按下，开始锁定目标数字...")

                if len(detections) > 0:
                    target_digit = choose_target_digit(detections)
                    last_sent_dir = ""
                    target_lock_time = now   # 记录锁定时间
                    msg = "TARGET={}\n".format(target_digit)

                    print("已锁定目标数字:", target_digit)
                    print("等待放回赛道，暂不判断方向...")
                    ## send_uart(msg)
                else:
                    print("当前画面没有识别到数字，锁定失败")
                    ## send_uart("TARGET=None\n")

                # 简单消抖
                time.sleep_ms(150)

            last_key_state = key_state

            # -------------------------------------------------
            # C. 如果已经锁定目标数字，则延时后持续寻找匹配数字
            # -------------------------------------------------
            if target_digit is not None:
                # 先等待一段时间，给你把数字放回赛道
                if time.ticks_diff(now, target_lock_time) < LOCK_DELAY_MS:
                    remain = LOCK_DELAY_MS - time.ticks_diff(now, target_lock_time)
                    print("等待中... 剩余 {} ms 后开始判断方向".format(remain))
                else:
                    match = find_matching_target(detections, target_digit)

                    if match is not None:
                        match_x = match['center_x']

                        # 调试输出：目标数字和匹配位置
                        pos_msg = "MATCH:{},X={:.1f},CENTER={:.1f}\n".format(
                            match['digit'], match_x, frame_center_x+CENTER_BIAS
                        )
                        print(pos_msg.strip())
                        ## send_uart(pos_msg)

                        # 左右判断
                        if time.ticks_diff(now, last_dir_send_time) > DIR_SEND_INTERVAL:
                            if match_x < frame_center_x+CENTER_BIAS:
                                if last_sent_dir != "L":
                                    print("方向标志: L，连续发送5次")
                                    send_uart_repeat("L\n", repeat=5, interval_ms=30)
                                    last_sent_dir = "L"
                                    last_dir_on_lcd = "L"
                                    last_dir_send_time = now

                            elif match_x > frame_center_x+CENTER_BIAS:
                                if last_sent_dir != "R":
                                    print("方向标志: R，连续发送5次")
                                    send_uart_repeat("R\n", repeat=5, interval_ms=30)
                                    last_sent_dir = "R"
                                    last_dir_on_lcd = "R"
                                    last_dir_send_time = now

                            else:
                                print("目标在正中间，不发送")
                    else:
                        # 当前帧没有找到目标数字，解除方向记忆
                        # 这样下次再次找到目标数字时，能重新发送一次L/R
                        last_sent_dir = ""

            # -------------------------------------------------
            # D. 画框显示
            # -------------------------------------------------
            pl.osd_img.clear()
            det_app.draw_result(pl.osd_img, res)

            # 在LCD左上角显示最近一次实际发送的方向
            if last_dir_on_lcd != "":
                pl.osd_img.draw_string_advanced(10, 10, 40, last_dir_on_lcd, color=(255, 0, 0))

            pl.show_image()
            gc.collect()

except KeyboardInterrupt:
    print("程序停止")

except Exception as e:
    print("主循环异常:", e)

finally:
    det_app.deinit()
    pl.destroy()
