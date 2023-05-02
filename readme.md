
# usb video device capture

## Introduction

    Media Foundationを用いてキャプチャしたUSBキャプチャデバイス/USBカメラ(Webカメラ)からの映像を  
    DirectX11ベースのレンダラーにて描画するプログラムです。  

## Performance

### Verification Machine Spec.

    ...

### Result

    ...

## Build

    ex. VS2017 の場合  
    powershell.exe cmake -S . -B build -G "\"Visual Studio 15 2017 Win64\""  
    powershell.exe cmake --build build  

    ex. VS2019以上の場合  
    powershell.exe cmake -S . -B build  
    powershell.exe cmake --build build  

    ex. Ninja + LLVMの場合(LLVM 16 win64で検証)  
    powershell.exe cmake -S . -B build -G "\"Ninja Multi-Config\""  
    powershell.exe cmake --build build --config debug

## How to use

    ...

