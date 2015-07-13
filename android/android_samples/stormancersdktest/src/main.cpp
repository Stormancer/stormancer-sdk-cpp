/* C++ Version */

#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <stormancer.h>

#define APPNAME "StormancerSdkTest"

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

std::shared_ptr<Stormancer::Scene> scene = nullptr;

extern "C"
{
    JNIEXPORT void JNICALL Java_com_stormancer_stormancertestsample_MainActivity_stormancerConnect(JNIEnv* env, jobject obj)
    {
		std::srand(time(NULL));

		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Create config");
		auto config = new Stormancer::Configuration("997bc6ac-9021-2ad6-139b-da63edee8c58", "base");
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Create client");
		auto client = new Stormancer::Client(config);
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");
		//std::shared_ptr<Scene> scene = nullptr;

		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Get scene");
		auto task = client->getPublicScene("main").then([/*&scene*/](std::shared_ptr<Stormancer::Scene> sc) {
			scene = sc;
			__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");
			int nbMsgToSend = 10;
			auto nbMsgReceived = new int(0);

			__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Add route");
			scene->addRoute("echo", [nbMsgToSend, nbMsgReceived](std::shared_ptr<Stormancer::Packet<Stormancer::IScenePeer>> p) {
				Stormancer::int32 number1, number2, number3;
				*p->stream >> number1 >> number2 >> number3;
				__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Received message: [ %d ; %d ; %d ]", number1, number2, number3);

				(*nbMsgReceived)++;
				if (*nbMsgReceived == nbMsgToSend)
				{
					__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");

					__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Disconnect");
					scene->disconnect().then([nbMsgReceived]() {
						__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");
						delete nbMsgReceived;
					});
				}
			});
			__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");

			__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Connect to scene");
			return scene->connect().then([nbMsgToSend]() {
				__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Done");
				for (int i = 0; i < nbMsgToSend; i++)
				{
					scene->sendPacket("echo", [](Stormancer::bytestream* stream) {
						Stormancer::int32 number1(rand()), number2(rand()), number3(rand());
						*stream << number1 << number2 << number3;
						__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Sending message: [ %d ; %d ; %d ]", number1, number2, number3);
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
