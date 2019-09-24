using System;
using System.Collections.Generic;
using System.Text;

namespace PluginSelector
{
    public class PluginList
    {
        public IEnumerable<Plugin> plugins { get; set; }
    }

    public class Plugin
    {
        public string name { get; set; }
    }
}
