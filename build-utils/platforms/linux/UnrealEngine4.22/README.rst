===========================================================
Building the Stormancer C++ Library for Linux (Unreal 4.22)
===========================================================

With Docker (recommended)
-------------------------

We provide a Docker image of a build environment that is specific to UE4.22.
Once deployed to a container, it can be targeted from Visual Studio to build the libraries.

We recommend this method because it provides a stable environment, and thus reproducible builds, nullifying possible issues of conflicts with system libraries.

Build and Run the Docker Image
******************************

We will assume you are using Ubuntu 18.04 on the machine that will host the container, but the same steps apply on any distribution. We will subsenquently refer to this machine as the **host machine**.

On the host machine, install docker if you haven't already (``sudo apt install docker.io``).

Copy the ``docker`` folder from this directory to the host machine.

On the host machine, ``cd`` to this folder, and run ``sudo docker build -t stormancer/ue422 .``.

Once the build is done, start the container using the following command::

	sudo docker run -d -p 12123:22 --security-opt seccomp:unconfined stormancer/ue422
	
Note the ``-p 12123:22``: this instructs Docker to map the port ``22`` of the container to the port ``12123`` on the host machine. This means you can SSH to the container via the port ``12123`` on the host machine. You can of course replace ``12123`` by any allowed (and free) port number.

The hash of the newly started container will be printed to the standard output. If you wish to stop the container, use ``sudo docker kill <hash>``.

Connect to the Container from Visual Studio
*******************************************

On the Windows machine on which you run Visual Studio, open it, and do the following:

Add the container as a remote build machine (``Tools > Options > Cross Platform > Connection Manager``): specify the IP of the host machine,
and the port that you gave as parameter to the ``docker run`` command earlier.
Choose ``Password`` authentication, using ``root`` as login and ``toor`` as password.

If you have more than one remote machine, make sure in the settings of the following projects that they are configured to target the newly added machine::
	
	cpprestsdk_Linux
	RakNet_Linux
	stormancer_cpp_tester_linux
	stormancer-sdk-cpp-lib-Linux
	
For each project, open the ``Properties`` dialog, and change the ``General > Remote Build Machine`` setting to the newly added machine, if needed.

You can now build the library.
Open a Visual Studio command prompt, navigate to the root of the C++ client library, and run ``.\build-utils\scripts\BuildLinuxUE422.bat``.

Without Docker
--------------

We will assume you are using Ubuntu 18.04 to build Stormancer.

1. Copy and extract ``docker/ue422sdk.7z`` to your Home folder on the Linux machine you will use to build Stormancer. Make sure you use the Home folder of the same account as the one you set for SSH access in Visual Studio settings.

2. From a command pompt, run ``sudo apt install clang-7 libc++-7-dev libc++abi-7-dev``.

3. On the Windows machine, open a Visual Studio command prompt, navigate to the root of the C++ client library, and run ``.\build-utils\scripts\BuildLinuxUE422.bat``.
