===========================================================
Building the Stormancer C++ Library for Linux (Unreal 4.22)
===========================================================

We will assume you are using Ubuntu 18.04 to build Stormancer.

1. Copy and extract ``ue422sdk.7z`` to your Home folder on the Linux machine you will use to build Stormancer. Make sure you use the Home folder of the same account as the one you set for SSH access in Visual Studio settings.

2. From a command pompt, run ``sudo apt install clang-7 libc++-7-dev libc++abi-7-dev``.

3. On the Windows machine, open a Visual Studio command prompt, navigate to the root of the C++ client library, and run ``.\scripts\BuildLinuxUE422.bat``.