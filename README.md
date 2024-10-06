## system_info

```
[ 11773][I][system_info.cpp:20] printSystemInfo(): [example:system_info] ESP32 Chip Information:
[ 11782][I][system_info.cpp:21] printSystemInfo(): [example:system_info] - Model: ESP32-D0WD-V3
[ 11793][I][system_info.cpp:22] printSystemInfo(): [example:system_info] - Cores: 2
[ 11802][I][system_info.cpp:24] printSystemInfo(): [example:system_info] - Features: WiFi/BT/BLE
[ 11813][I][system_info.cpp:25] printSystemInfo(): [example:system_info] - Silicon revision: 3  
[ 11824][I][system_info.cpp:30] printSystemInfo(): [example:system_info] Flash Memory: 4MB external
[ 11835][I][system_info.cpp:33] printSystemInfo(): [example:system_info] CPU Frequency: 240MHz  
[ 11846][I][system_info.cpp:36] printSystemInfo(): [example:system_info] SRAM Size: 365 KB
[ 11856][I][system_info.cpp:37] printSystemInfo(): [example:system_info] Available SRAM: 342 KB
[ 11866][I][system_info.cpp:43] printSystemInfo(): [example:system_info] MAC Address: 08:A6:F7:48:70:AC
[ 11878][E][system_info.cpp:70] printSystemInfo(): [example:system_info] Failed to get WiFi mode  
[ 11889][I][system_info.cpp:77] printSystemInfo(): [example:system_info] Battery Voltage: 0.00V
[ 11899][I][system_info.cpp:80] printSystemInfo(): [example:system_info] System Uptime: 11 seconds
```

## heavy_task_serial

```
[ 39164][I][heavy_task_serial.cpp:16] performHeavyTask(): [example:heavy_task_serial] Task1 completed. Result: 1783293664
[ 39225][I][heavy_task_serial.cpp:16] performHeavyTask(): [example:heavy_task_serial] Task2 completed. Result: 1783293664
[ 39236][I][heavy_task_serial.cpp:36] loop(): [example:heavy_task_serial] Total execution time: 122 ms
[ 39248][I][heavy_task_serial.cpp:37] loop(): [example:heavy_task_serial] --------------------
```

## heavy_task_parallel

```
[ 10230][I][heavy_task_parallel.cpp:23] performHeavyTa[s k1(0)2:3 0[]e[xIa]m[phleea:vhye_atvays_kt_apsakr_aplalreall.lceplp]: 2T3]sk2rcorpmetavyonscore [e Replet: 1avy2tas64parallel] Task1 completed on core 0. Result: 1783293664
[ 10255][I][heavy_task_parallel.cpp:57] loop(): [example:heavy_task_parallel] Total execution time: 76 ms
[ 10265][I][heavy_task_parallel.cpp:58] loop(): [example:heavy_task_parallel] --------------------
```

> notice mangled log by race condition

## heavy_task_parallel_mutex

```
Task2 completed on core 1. Result: 1783293664
Task1 completed on core 0. Result: 1783293664
Total execution time: 54 ms
--------------------
```

## math_fft_naive

```
[  6958][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 0: 0.0605
[  6967][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 1: 0.4898
[  6979][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 2: 0.3238
[  6990][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 3: 0.2472
[  7001][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 4: 0.1167
[  7012][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 5: 0.1268
[  7024][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 6: 0.0734
[  7035][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 7: 0.1736
[  7046][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 8: 0.1471
[  7057][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 9: 0.0386
[  7069][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 10: 4064.5164
[  7080][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 11: 0.0411
[  7092][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 12: 0.0722
[  7103][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 13: 0.0146
[  7114][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 14: 0.2591
[  7126][I][math_fft_naive.cpp:48] heavy_task(): [example:math_fft_naive] Power at frequency 15: 0.5501
[  7137][I][math_fft_naive.cpp:51] heavy_task(): [example:math_fft_naive] Time taken: 1629 milliseconds
```

## math_fft_dsp

```
[  4888][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 0: 0.0605
[  4897][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 1: 1.9593
[  4908][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 2: 1.2951
[  4919][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 3: 0.9889
[  4930][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 4: 0.4669
[  4940][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 5: 0.5072
[  4951][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 6: 0.2935
[  4962][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 7: 0.6943
[  4973][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 8: 0.5883
[  4984][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 9: 0.1544
[  4995][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 10: 16258.0693
[  5006][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 11: 0.1645
[  5017][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 12: 0.2890
[  5028][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 13: 0.0584
[  5039][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 14: 1.0364
[  5050][I][math_fft_dsp.cpp:48] heavy_task(): [example:math_fft_dsp] Power at frequency 15: 2.2005
[  5061][I][math_fft_dsp.cpp:51] heavy_task(): [example:math_fft_dsp] Time taken: 0.1860 milliseconds
```

> 1000x faster? the number is highly suspicious..

## math_dotprod_naive

```
[  3536][I][math_dotprod_naive.cpp:45] heavy_task(): [example:heavy_task_parallel] Dot product result = 260.929718
[  3546][I][math_dotprod_naive.cpp:46] heavy_task(): [example:heavy_task_parallel] Operation took 13468 cycles
[  3558][I][math_dotprod_naive.cpp:47] heavy_task(): [example:heavy_task_parallel] Operation took 56 microseconds
```

## math_dotprod_dsp

```
[  3540][I][math_dotprod_dsp.cpp:44] heavy_task(): [example:heavy_task_parallel] Dot product result = 255.693115
[  3550][I][math_dotprod_dsp.cpp:45] heavy_task(): [example:heavy_task_parallel] Operation took 4253 cycles
[  3562][I][math_dotprod_dsp.cpp:46] heavy_task(): [example:heavy_task_parallel] Operation took 17 microseconds
```

## camera_throughput

PIXFORMAT_JPEG, FRAMESIZE_QQVGA
Throughput: 50.00 FPS, Avg size of last 5 images: 3065.00 bytes

PIXFORMAT_JPEG, FRAMESIZE_QVGA
Throughput: 50.00 FPS, Avg size of last 5 images: 3065.00 bytes

PIXFORMAT_GRAYSCALE

## converter_throughput

QQVGA gray to rgb
Current FPS: 177.62

QQVGA gray to jpeg:
```
Current FPS: 101.14, JPEG size: 2481 bytes
Free heap: 337496 bytes
Largest free block: 3997684 bytes
Free PSRAM: 4041819 bytes
```

QVGA gray to jpeg:
```
Current FPS: 25.36, JPEG size: 7345 bytes
Free heap: 337496 bytes
Largest free block: 3932148 bytes
Free PSRAM: 3984219 bytes
```

### Supported Sensor

most of them capped at 12.5 fps even in qqvga. only jpeg give you 50 fps
even QQVGA + grayscale is capped at 12.5 fps.
OV2640:

PIXFORMAT_RGB565,    // fails, 1, 0.6
PIXFORMAT_YUV422,    // 
PIXFORMAT_YUV420,    // 
PIXFORMAT_GRAYSCALE, // 
PIXFORMAT_JPEG,      // 
PIXFORMAT_RGB888,    // 
PIXFORMAT_RAW,       // 
PIXFORMAT_RGB444,    // 
PIXFORMAT_RGB555,    // 


| model   | max resolution | color type | output format                                                | Len Size |
| ------- | -------------- | ---------- | ------------------------------------------------------------ | -------- |
| OV2640  | 1600 x 1200    | color      | YUV(422/420)/YCbCr422<br>RGB565/555<br>8-bit compressed data<br>8/10-bit Raw RGB data | 1/4"     |
| OV3660  | 2048 x 1536    | color      | raw RGB data<br/>RGB565/555/444<br/>CCIR656<br/>YCbCr422<br/>compression | 1/5"     |
| OV5640  | 2592 x 1944    | color      | RAW RGB<br/>RGB565/555/444<br/>CCIR656<br/>YUV422/420<br/>YCbCr422<br/>compression | 1/4"     |
| OV7670  | 640 x 480      | color      | Raw Bayer RGB<br/>Processed Bayer RGB<br>YUV/YCbCr422<br>GRB422<br>RGB565/555 | 1/6"     |