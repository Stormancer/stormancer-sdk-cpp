using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace PluginSelector
{
    public class PluginSelectorTask : Task
    {
        public override bool Execute()
        {
            try
            {
                var selector = new PluginSelector(PluginListFile, PluginsDirectory);

                var plugins = selector.GetChosenPlugins();

                PluginSourceFiles = plugins.SourceFiles.Select(file => new TaskItem(file)).ToArray();
                PluginPrivateHeaderFiles = plugins.PrivateHeaderFiles.Select(file => new TaskItem(file)).ToArray();
                PluginPublicHeaderFiles = plugins.PublicHeaderFiles.Select(file => new TaskItem(file.HeaderFilename, new Dictionary<string, string>() { { "PluginName", file.PluginName } })).ToArray();
            }
            catch (Exception e)
            {
                Log.LogError("PluginSelector error: " + e.Message);
                return false;
            }
            return true;
        }

        [Required]
        public string PluginsDirectory { get; set; }

        [Required]
        public string PluginListFile { get; set; }

        [Output]
        public ITaskItem[] PluginSourceFiles { get; set; }

        [Output]
        public ITaskItem[] PluginPublicHeaderFiles { get; set; }

        [Output]
        public ITaskItem[] PluginPrivateHeaderFiles { get; set; }
    }
}
