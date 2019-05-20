using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace PluginSelector
{
    class PluginSelector
    {
        private string jsonFile;
        private string pluginFolderPath;

        public PluginSelector(string jsonPluginList, string pluginFolderPath)
        {
            this.jsonFile = jsonPluginList;
            this.pluginFolderPath = pluginFolderPath;
        }

        public ChosenPlugins GetChosenPlugins()
        {
            var plugins = JsonConvert.DeserializeObject<PluginList>(File.ReadAllText(jsonFile));

            var pluginsPrivateFiles = GatherPluginPrivateFiles(plugins);

            var pluginPublicHeaders = GatherPluginHeaderFiles(plugins);

            return new ChosenPlugins
            {
                PublicHeaderFiles = pluginPublicHeaders,
                SourceFiles = pluginsPrivateFiles.SourceFiles,
                PrivateHeaderFiles = pluginsPrivateFiles.PrivateHeaderFiles
            };
        }

        private PluginPrivateFiles GatherPluginPrivateFiles(PluginList plugins)
        {
            var files = new PluginPrivateFiles();

            foreach (var plugin in plugins.plugins)
            {
                var pluginPath = Path.Combine(pluginFolderPath, "private", plugin.name);
                files.SourceFiles.AddRange(Directory.GetFiles(pluginPath, "*.cpp"));
                files.PrivateHeaderFiles.AddRange(Directory.GetFiles(pluginPath, "*.h"));
            }

            return files;
        }

        private IEnumerable<PluginPublicHeader> GatherPluginHeaderFiles(PluginList plugins)
        {
            List<PluginPublicHeader> files = new List<PluginPublicHeader>();

            foreach (var plugin in plugins.plugins)
            {
                var pluginPath = Path.Combine(pluginFolderPath, "public", plugin.name);
                files.AddRange(Directory.GetFiles(pluginPath, "*.h").Select(file => new PluginPublicHeader { PluginName = plugin.name, HeaderFilename = file }));
            }

            return files;
        }
    }
}
