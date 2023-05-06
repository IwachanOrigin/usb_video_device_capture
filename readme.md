
# usb video device capture

## Introduction

    Media Foundationを用いてキャプチャしたUSBキャプチャデバイス/USBカメラ(Webカメラ)からの映像を  
    DirectX11ベースのレンダラーにて描画するプログラムです。  

## Performance

### Verification Machine Spec

    OS  : Windows 10 Pro 21H2
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

| No. | Device Name                 | Max Resolution | MAX FPS | Audio support |
| 1   | Logicool HD Pro Webcam C920 | 2K(1920x1080)  | 60p     | Yes           |
| 2   | I-O DATA GV-HUVC 4K         | 4K(3840x2160)  | 30p     | Yes           |

## Build

    ex. VS2017 の場合  
    powershell.exe cmake -S . -B build -G "\"Visual Studio 15 2017 Win64\""  
    powershell.exe cmake --build build  

    ex. VS2019以上の場合  
    powershell.exe cmake -S . -B build  
    powershell.exe cmake --build build  

## How to use

    ...

