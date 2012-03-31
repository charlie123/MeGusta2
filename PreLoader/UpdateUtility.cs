using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Diagnostics;
using System.IO;
using System.Threading;
using DBNUpdater;

namespace aIW
{
    public static class UpdateUtility
    {
        static bool completed = false;

        public static bool Bootstrap()
        {
            if (Directory.Exists("bootstrap") && File.Exists(@"bootstrap\iw4mp.exe"))
            {
                //return;
            }

            Directory.CreateDirectory("bootstrap");

            var _core = new Core();
            _core.CacheLocation = Path.Combine(Environment.CurrentDirectory, "bootstrap");
            //_core.WebCacheServer = "http://localhost/ctest_aiwbs/diff/";
            _core.WebCacheServer = "http://alteriw.net/updater/bootstrap/";
            _core.LocalCacheServer = "";
            _core.WantedCaches = new[] { "aiw-updater" };

            var updateScript = Path.Combine(_core.CacheLocation, "update.cmd");

            // delete the update script
            // it'll be re-generated if an update is available, so don't worry. :)
            File.Delete("update.cmd");
            File.Delete(updateScript);

            _core.Start();

            // wait for completion
            completed = false;

            _core.Completed += (s, e) =>
            {
                UpdateUtility.completed = true;
            };

            _core.Failed += (s, e) =>
            {
                UpdateUtility.completed = true;
            };

            while (!completed)
            {
                Thread.Sleep(1);
            }

            // run the update script if it was updated
            if (File.Exists(updateScript))
            {
                var psi = new ProcessStartInfo(updateScript, "iw4mp.exe");
                psi.WorkingDirectory = Environment.CurrentDirectory;

                Process.Start(psi);

                return true;
            }

            return false;
        }
    }
}
