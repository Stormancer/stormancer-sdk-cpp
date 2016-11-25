using System;
using System.IO;
using System.Text.RegularExpressions;

namespace UnrealBuildTool.Rules
{
	public class StormancerPlugin : ModuleRules
	{
		private const string VS_TOOLSET = "140";
		public StormancerPlugin(TargetInfo target)
		{
			/** Setup an external C++ module */
			LoadLibrary(target, "3rdparty/stormancer/include", "3rdparty/stormancer/libs", "stormancer");

			/** Setup files in this plugin locally */
			SetupLocal(target);
		}


		/** Perform all the normal module setup for plugin local c++ files. */
		private void SetupLocal(TargetInfo target) {
			PublicIncludePaths.AddRange(new string[] {"source/Stormancer/Public" });
			PrivateIncludePaths.AddRange(new string[] {"Source/Stormancer/Private" });
			PublicDependencyModuleNames.AddRange(new string[] {"Core"});
			PrivateDependencyModuleNames.AddRange(new string[] {});
			DynamicallyLoadedModuleNames.AddRange(new string[] {});
		}

		/**
		 * Helper to setup an arbitrary library in the given library folder
		 * @param include_path Relative include path, eg. 3rdparty/mylib/include
		 * @param build_path Relative build path, eg. 3rdparty/mylib/build
		 * @param library_name Short library name, eg. mylib. Automatically expands to libmylib.a, mylib.lib, etc.
		 */
		private void LoadLibrary(TargetInfo target, string include_path, string build_path, string library_name) {

			// Add the include path
			var full_include_path = Path.Combine(PluginPath, include_path);
			if (!Directory.Exists(full_include_path)) {
				Fail("Invalid include path: " + full_include_path);
			}
			else {
				PublicIncludePaths.Add(full_include_path);
				Trace("Added include path: {0}", full_include_path);
			}

			// Get the build path
			var full_build_path = Path.Combine(PluginPath, build_path);
			if (!Directory.Exists(full_build_path)) {
				Fail("Invalid build path: " + full_build_path + " (Did you build the 3rdparty module already?)");
			}

			string platform = "";
			switch(target.Platform)
			{
				case UnrealTargetPlatform.Win64:
				platform = "x64";
				break;
				case UnrealTargetPlatform.Win32:
				platform = "x86";
				break;
				default:
				break;
			}
			// Look at all the files in the build path; we need to smartly locate
			// the static library based on the current platform. For dynamic libraries
			// this is more difficult, but for static libraries, it's just .lib or .a
			string [] fileEntries = Directory.GetFiles(full_build_path);
			var pattern = ".*" + library_name + VS_TOOLSET+"_Release_"+platform+".";
			if ((target.Platform == UnrealTargetPlatform.Win64) || (target.Platform == UnrealTargetPlatform.Win32)) {
				pattern += "lib";
			}
			else {
				pattern += "a";
			}
			Regex r = new Regex(pattern, RegexOptions.IgnoreCase);
			string full_library_path = null;
			foreach (var file in fileEntries) {
				if (r.Match(file).Success) {
					full_library_path = Path.Combine(full_build_path, file);
					break;
				}
			}
			if (full_library_path == null) {
				Fail("Unable to locate any build libraries in: " + full_build_path);
			}

			// Found a library; add it to the dependencies list
			PublicAdditionalLibraries.Add(full_library_path);
			Trace("Added static library: {0}", full_library_path);
		}

		/**
		 * Print out a build message
		 * Why error? Well, the UE masks all other errors. *shrug*
		 */
		private void Trace(string msg) {
			Log.TraceError(Plugin + ": " + msg);
		}

		/** Trace helper */
		private void Trace(string format, params object[] args) {
			Trace(string.Format(format, args));
		}

		/** Raise an error */
		private void Fail(string message) {
			Trace(message);
			throw new Exception(message);
		}

		/** Get the absolute root to the plugin folder */
		private string PluginPath {
			get {
				return Path.GetFullPath(Path.Combine(ModuleDirectory, "../.."));
			}
		}

		/** Get the name of this plugin's folder */
		private string Plugin {
			get {
				return new DirectoryInfo(PluginPath).Name;
			}
		}
	}
}