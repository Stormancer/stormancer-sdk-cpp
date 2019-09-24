using System;
using System.Collections.Generic;
using System.Text;

namespace PluginSelector
{
    public class ChosenPlugins
    {
        public IEnumerable<string> SourceFiles { get; set; }

        public IEnumerable<string> PrivateHeaderFiles { get; set; }

        public IEnumerable<PluginPublicHeader> PublicHeaderFiles { get; set; }
    }

    public class PluginPublicHeader
    {
        public string PluginName { get; set; }

        public string HeaderFilename { get; set; }
    }
}
