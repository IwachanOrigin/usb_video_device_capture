
# usb video device capture

## Introduction

    Media Foundationを用いてキャプチャしたUSBキャプチャデバイス/USBカメラ(Webカメラ)からの映像を  
    DirectX11ベースのレンダラーにて描画するプログラムです。  

## Demo
![Demo](https://github.com/IwachanOrigin/usb_video_device_capture/blob/7e322b10cec8068fdbce67ee17c9daf15a2a1a02/doc/images/demo.gif)

## Performance

### Verification Machine Spec

#### No.1  

    OS  : Windows 10 Pro 22H2  
    CPU : Intel Core i9-7900X(10core, 20threads)  
    RAM : 24.0GB  
    GPU : NVIDIA GeForce RTX A4000(Driver ver.535.98)  

#### No.2  

    OS  : Windows 10 Pro 22H2  
    CPU : Intel Pentium N4200(4core, 4threads)  
    RAM : 4.0GB  
    GPU : Intel(R) HD Graphics 505  

### Verification USB Web camera

    Logicool HD Pro Webcam C920  
[LINK](https://www.logicool.co.jp/ja-jp/products/webcams/hd-pro-webcam-c920n.960-001261.html)

### Verification USB Video Capture Device

    I-O DATA GV-HUVC 4K  
[LINK](https://www.iodata.jp/product/av/capture/gv-huvc4k/index.htm)

### Result

| No. | Device Name                 | MAX Resolution | MAX FPS | Audio support |
|-----|-----------------------------|----------------|---------|---------------|
| 1   | Logicool HD Pro Webcam C920 | 2K(1920x1080)  | 30p     | Yes           |
| 2   | I-O DATA GV-HUVC 4K         | 4K(3840x2160)  | 30p     | Yes           |

#### Logicool HD Pro Webcam C920(Input : 2K30p, Output : 2K30p) <CPU, Memory, Disk, NW, GPU>

    - With No.1 PC(DMO)  
![input_output_2k30p_no01_dmo](./doc/images/input_output_2k30p_no01_dmo.png)

    - With No.1 PC(Pixel Shader)  
![input_output_2k30p_no01_ps](./doc/images/input_output_2k30p_no01_ps.png)

    - With No.2 PC(DMO)  
![input_output_2k30p_no02_dmo](./doc/images/input_output_2k30p_no02_dmo.png)

    - With No.2 PC(Pixel Shader)  
![input_output_2k30p_no02_ps](./doc/images/input_output_2k30p_no02_ps.png)

#### I-O DATA GV-HUVC 4K(Input : 4K30p, Output : 4K30p) <CPU, Memory, Disk, NW, GPU>

    - With No.1 PC(DMO)  
![input_output_4k30p_no01_dmo](./doc/images/input_output_4k30p_no01_dmo.png)

    - With No.1 PC(Pixel Shader)  
![input_output_4k30p_no01_ps](./doc/images/input_output_4k30p_no01_ps.png)

    - With No.2 PC(DMO)
![input_output_4k30p_no02_dmo](./doc/images/input_output_4k30p_no02_dmo.png)

    - With No.2 PC(Pixel Shader)  
![input_output_4k30p_no02_ps](./doc/images/input_output_4k30p_no02_ps.png)

## Build

    ex. For VS2017  
    powershell.exe cmake -S . -B build -G "\"Visual Studio 15 2017 Win64"\"  
    powershell.exe cmake --build build  

    ex. VS2019 or higher  
    powershell.exe cmake -S . -B build  
    powershell.exe cmake --build build  

## How to use

    1. Run the usbVideoDeviceCapture.exe from console.  
    2. Select the video capture device number.  
    3. Select the audio capture device number.  
    4. Select the audio output device number.  
    5. Select the color conversion mode number.  
    6. A window will appear and capture will begin.  

![how_to_use](https://github.com/IwachanOrigin/usb_video_device_capture/blob/1fa0ac7ecec934c6080774b6735a5660707b00aa/doc/images/how_to_use.png)

## Note

    - If all settings good, but capture is not started, the active signal resolution is not right.  
      Please check your windows display settings.  
      for example...  
![active_signal_resolution](https://github.com/IwachanOrigin/usb_video_device_capture/blob/1fa0ac7ecec934c6080774b6735a5660707b00aa/doc/images/active_signal_resolution.png)

## Supported color format

### Color Converter DSP(DirectX Media Object)

Please check to below url.  
[https://learn.microsoft.com/ja-jp/windows/win32/medfound/colorconverter](https://learn.microsoft.com/ja-jp/windows/win32/medfound/colorconverter)  

### Pixel Shader

| No. | Color format |
|-----|--------------|
| 1   | NV12         |
| 2   | RGB32        |

## Design

![design](https://github.com/IwachanOrigin/usb_video_device_capture/blob/9dcf10dcd3e06da4cf24942abb2c5eb694b5099f/doc/images/design.png)

