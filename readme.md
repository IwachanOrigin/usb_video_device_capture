
# usb video device capture

## Introduction

    Media Foundationを用いてキャプチャしたUSBキャプチャデバイス/USBカメラ(Webカメラ)からの映像を  
    DirectX11ベースのレンダラーにて描画するプログラムです。  

## Performance

### Verification Machine Spec

    OS  : Windows 10 Pro 22H2
    CPU : Intel Core i7-9700K
    RAM : 32.0GB
    GPU : NVIDIA GeForce RTX 2060(Driver ver.522.30)

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

    - Logicool HD Pro Webcam C920(Input : 2K30p, Output : 2K30p) <CPU, Memory, Disk, NW, GPU>
![input_output_2k30p](https://github.com/IwachanOrigin/usb_video_device_capture/blob/eb5f8f47a906f8a32875bbc672d39c4b7e0965cb/doc/images/input_output_2k30p.png)

    - I-O DATA GV-HUVC 4K(Input : 4K30p, Output : 4K30p) <CPU, Memory, Disk, NW, GPU>
![input_4k30p_output_4k30p](https://github.com/IwachanOrigin/usb_video_device_capture/blob/1c87211e987152a1df2e078f34ca713460537169/doc/images/input_output_4k30p.png)

## Build

    ex. VS2017 の場合  
    powershell.exe cmake -S . -B build -G "\"Visual Studio 15 2017 Win64\""  
    powershell.exe cmake --build build  

    ex. VS2019以上の場合  
    powershell.exe cmake -S . -B build  
    powershell.exe cmake --build build  

## How to use

    1. Run the usbVideoDeviceCapture.exe  
    2. Select the video capture device number.  
    3. Select the audio capture device number.  
    4. Select the audio output device number.  
    5. A window will appear and capture will begin.  

![how_to_use](https://github.com/IwachanOrigin/usb_video_device_capture/blob/1fa0ac7ecec934c6080774b6735a5660707b00aa/doc/images/how_to_use.png)

## Attention

    - If all settings good, but capture is not started, the active signal resolution is not right.  
      Please check your windows display settings.  
      for example...  
![active_signal_resolution](https://github.com/IwachanOrigin/usb_video_device_capture/blob/1fa0ac7ecec934c6080774b6735a5660707b00aa/doc/images/active_signal_resolution.png)


## Design

![design](https://github.com/IwachanOrigin/usb_video_device_capture/blob/9dcf10dcd3e06da4cf24942abb2c5eb694b5099f/doc/images/design.png)
