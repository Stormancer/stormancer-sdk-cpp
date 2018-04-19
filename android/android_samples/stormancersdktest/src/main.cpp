/* C++ Version */

#define APPNAME "stormancersdktest"

#include <string.h>
#include <jni.h>
#include <stormancer.h>
#include "AndroidLogger.h"

/* 
* replace com_example_whatever with your package name
*
* HelloJni should be the name of the activity that will 
* call this function
*
* change the returned string to be one that exercises
* some functionality in your wrapped library to test that
* it all works
*
*/

auto logger = (AndroidLogger*)Stormancer::ILogger::instance(new AndroidLogger(Stormancer::LogLevel::Trace));

std::shared_ptr<Stormancer::Scene> scene = nullptr;

extern "C"
{
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		JNIEnv* env;
		if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
		{
			return -1;
		}

		cpprest_init(vm);
		return JNI_VERSION_1_6;
	}

    JNIEXPORT void JNICALL Java_com_stormancer_stormancertestsample_MainActivity_stormancerConnect(JNIEnv* env, jobject obj)
    {
		std::srand(time(NULL));

		logger->log("Create config");
		auto config = new Stormancer::Configuration("997bc6ac-9021-2ad6-139b-da63edee8c58", "base");
		logger->log("Done");
		logger->log("Create client");
		auto client = new Stormancer::Client(config);
		logger->log("Done");

		logger->log("Get scene");
		auto task = client->getPublicScene("main").then([/*&scene*/](std::shared_ptr<Stormancer::Scene> sc) {
			scene = sc;
			logger->log("Done");
			int nbMsgToSend = 10;
			auto nbMsgReceived = new int(0);

			logger->log("Add route");
			scene->addRoute("echo", [nbMsgToSend, nbMsgReceived](std::shared_ptr<Stormancer::Packet<Stormancer::IScenePeer>> p) {
				Stormancer::int32 number1, number2, number3;
				*p->stream >> number1 >> number2 >> number3;
				std::stringstream ss;
				ss << "Received message: [ " << number1  << " ; " << number2 << " ; " << number3 << " ]";
				logger->log(ss.str());

				(*nbMsgReceived)++;
				if (*nbMsgReceived == nbMsgToSend)
				{
					logger->log("Done");

					logger->log("Disconnect");
					scene->disconnect().then([nbMsgReceived]() {
						logger->log("Done");
						delete nbMsgReceived;
					});
				}
			});
			logger->log("Done");

			logger->log("Connect to scene");
			return scene->connect().then([nbMsgToSend]() {
				logger->log("Done");
				for (int i = 0; i < nbMsgToSend; i++)
				{
					scene->sendPacket("echo", [](Stormancer::bytestream* stream) {
						Stormancer::int32 number1(rand()), number2(rand()), number3(rand());
						*stream << number1 << number2 << number3;

						std::stringstream ss;
						ss << "Sending message: [ " << number1  << " ; " << number2 << " ; " << number3 << " ]";
						logger->log(ss.str());
					});
				}
			});
		});
    }

    JNIEXPORT bool JNICALL Java_com_stormancer_stormancertestsample_MainActivity_stormancerConnected(JNIEnv* env, jobject obj)
    {
        return false;
    }

    JNIEXPORT jstring JNICALL Java_com_stormancer_stormancertestsample_MainActivity_stormancerGetString(JNIEnv* env, jobject obj)
    {
        return env->NewStringUTF("My Stormancer String !");
    }
}
