using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Reflection;
using System.IO;

namespace CitizenLauncher
{
    public static class Config
    {
        private static Dictionary<string, string> Settings { get; set; }

        static Config()
        {
            Settings = new Dictionary<string, string>();

            Read();
        }

        public static string Get(string setting, string defaultValue)
        {
            if (Settings.ContainsKey(setting))
            {
                return Settings[setting].Trim();
            }

            return defaultValue;
        }

        public static void Read()
        {
            var assemblyName = Assembly.GetExecutingAssembly().GetName().Name;
            var configFiles = new[] {
                @".\" + assemblyName + ".cfg"
            };

            var configFile = "";
            foreach (var fileN in configFiles)
            {
                if (File.Exists(fileN))
                {
                    configFile = fileN;
                    break;
                }
            }

            if (configFile == "")
            {
                return;
            }

            var file = File.OpenText(configFile);

            while (!file.EndOfStream)
            {
                var line = Regex.Replace(file.ReadLine(), "(\\s|,)+", " ");

                if (line.Trim() == "")
                {
                    continue;
                }

                if (line[0] == '#' || line[0] == ';')
                {
                    continue;
                }

                var data = line.Split(new[] { ' ' }, 2);

                if (data.Length < 2)
                {
                    continue;
                }

                Settings.Add(data[0], data[1]);
            }

            file.Close();
        }
    }
}
