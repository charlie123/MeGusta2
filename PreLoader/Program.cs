using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;
using System.Net;
using System.Security.Cryptography;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using System.Web;
using System.Xml.Linq;

using CitizenLauncher;
using DBNUpdater;

namespace DBNetwork.Citizen.Launcher
{
    static class Program
    {
        static bool openUpdater;
        public static bool Completed { get; set; }

        static void Main(string[] args)
        {
            try
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                Environment.CurrentDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);//Application.StartupPath;

                try
                {
                    var str = File.Create(Environment.CurrentDirectory + @"\test.log");
                    str.Close();

                    Directory.CreateDirectory(Environment.CurrentDirectory + @"\test");

                    Directory.Delete(Environment.CurrentDirectory + @"\test");
                    File.Delete(Environment.CurrentDirectory + @"\test.log");
                }
                catch (UnauthorizedAccessException)
                {
                    if (!Program.ModifyRights())
                    {
                        return;
                    }
                }

                File.Delete("__iw4mp");

                var files = Directory.GetFiles(Environment.CurrentDirectory, "*.exe", SearchOption.TopDirectoryOnly);
                foreach (var file in files)
                {
                    if (Regex.Match(file.Split('.')[0], "([0-9A-F]{7,10})").Success)
                    {
                        File.Delete(file);
                    }
                }

                CheckCaches(@"bootstrap\caches.xml");

                if (aIW.UpdateUtility.Bootstrap())
                {
                    return;
                }

                if (!File.Exists("mss32.dll") || !File.Exists("binkw32.dll") || !File.Exists(@"main\iw_00.iwd"))
                {
                    MessageBox.Show("Some game library files could not be found. It could be you didn't put the updater files in your existing Modern Warfare 2 game folder. Also, you still need the game if you want to run alterIWnet.", "alterIWnet", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                ConnectionManager.Install();

                var processes = Process.GetProcessesByName("iw4mp");
                if (processes.Length > 1)
                {
                    ConnectionManager.Handle(args, true);
                    return;
                }

                processes = Process.GetProcessesByName("iw4mp.dat");
                if (processes.Length > 0)
                {
                    ConnectionManager.Handle(args, true);
                    return;
                }

                // pre-run task: remove improper mp_playlists.ff files (english)
                // added 2010-07-23

                if (File.Exists(@"zone\english\mp_playlists.ff"))
                {
                    // delete it in all cases so it updates
                    File.Delete(@"zone\english\mp_playlists.ff");

                    // old code
                    /*
                    var info = new FileInfo(@"zone\english\mp_playlists.ff");
                    if (info.Length < 16384)
                    { // 16 kB should be a proper minimum size
                        File.Delete(@"zone\english\mp_playlists.ff");
                    }
                    */
                }

                var enableUpdates = Config.Get("enable_updates", "1");

                if (enableUpdates == "0")
                {
                    goto skipUpdate;
                }

                CheckCaches(@"caches.xml");

                var icon = new NotifyIcon();
                icon.Icon = System.Drawing.Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location);
                icon.Text = "alterIWnet - starting";
                icon.Visible = true;

                var _core = new Core();
                _core.CacheLocation = Environment.CurrentDirectory;
                _core.WebCacheServer = Config.Get("cache_server", "http://localhost/ctest_aiw2/");
                _core.LocalCacheServer = Config.Get("local_cache", "");
                _core.WantedCaches = Config.Get("caches", "citizen").Split(';');
                _core.EnableUploading = (Config.Get("enable_upload", "1") != "0");

                //_core.Completed += new EventHandler<EventArgs>(_core_Completed);
                //_core.Failed += new EventHandler<FailedEventArgs>(_core_Failed);
                //_core.StatusChanged += new EventHandler<StatusChangedEventArgs>(_core_StatusChanged);

                _core.StatusChanged += new EventHandler<StatusChangedEventArgs>(_core_StatusChanged);
                _core.Failed += new EventHandler<FailedEventArgs>(_core_Failed);
                _core.Completed += new EventHandler<EventArgs>(_core_Completed);

                _core.Start();

                var time = DateTime.UtcNow;
                var shown = false;

                while (!Completed)
                {
                    System.Threading.Thread.Sleep(1);

                    var newTime = DateTime.UtcNow;
                    var diff = newTime - time;

                    if (diff.TotalMilliseconds > 2000) {
                        if (!shown)
                        {
                            icon.ShowBalloonTip(2500, "alterIWnet", "alterIWnet is currently verifying data files. Don't worry, it'll start soon. ;)", ToolTipIcon.Info);

                            shown = true;
                        }
                    }
                }

                if (openUpdater)
                {
                    var updater = new frmUpdater(_core);
                    Application.Run(updater);
                }

                if (!frmUpdater.Finished)
                {
                    return;
                }

                icon.Visible = false;
                icon.Dispose();

            skipUpdate:
                var assemblyName = Assembly.GetExecutingAssembly().GetName().Name;
                var progFile = assemblyName + ".dat";

                if (!File.Exists(Environment.CurrentDirectory + @"\" + progFile))
                {
                    MessageBox.Show(progFile + " does not exist.");
                    return;
                }

                /*var random = new Random();
                var name = random.Next(0x1000000, 0xFFFFFFF).ToString("X7") + ".exe";

                File.Copy(progFile, name);
                File.SetCreationTime(name, DateTime.Now - new TimeSpan(random.Next(3, 5), random.Next(1, 24), random.Next(1, 60), random.Next(1, 60)));
                File.SetLastWriteTime(name, DateTime.Now - new TimeSpan(random.Next(1, 3), random.Next(1, 24), random.Next(1, 60), random.Next(1, 60)));*/

                STARTUPINFO si = new STARTUPINFO();
                PROCESS_INFORMATION pi = new PROCESS_INFORMATION();
                CreateProcess(progFile, null, IntPtr.Zero, IntPtr.Zero, false, 0, IntPtr.Zero, null, ref si, out pi);

                var proc = Process.GetProcessById((int)pi.dwProcessId);

                ConnectionManager.Handle(args, false);

                proc.WaitForExit();

                //File.Delete(name);
                //var proc = new Process();
                //proc.
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString(), "alterIWnet", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        [DllImport("kernel32.dll")]
        static extern bool CreateProcess(string lpApplicationName, string lpCommandLine, IntPtr lpProcessAttributes, IntPtr lpThreadAttributes,
                                bool bInheritHandles, uint dwCreationFlags, IntPtr lpEnvironment,
                                string lpCurrentDirectory, ref STARTUPINFO lpStartupInfo, out PROCESS_INFORMATION lpProcessInformation);

        public static bool ModifyRights()
        {
            if (Environment.OSVersion.Version.Major >= 6)
            {
                var proc = new System.Diagnostics.Process();
                proc.StartInfo = new System.Diagnostics.ProcessStartInfo(Environment.CurrentDirectory + @"\zone\fixpermissions.exe");
                proc.StartInfo.WorkingDirectory = Environment.CurrentDirectory;
                proc.Start();
                proc.WaitForExit();

                return true;
            }
            else
            {
                MessageBox.Show("The MW2 directory is not writable. Please change the permissions or log on as an Administrator user and try again.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
        }

        private static bool IsRunning()
        {
            return Process.GetProcessesByName("iw4mp").Length > 0;
        }

        static void _core_Completed(object sender, EventArgs e)
        {
            frmUpdater.Finished = true;
            Completed = true;
        }

        static void _core_Failed(object sender, FailedEventArgs e)
        {
            MessageBox.Show(e.Exception.ToString(), "alterIWnet", MessageBoxButtons.OK, MessageBoxIcon.Error);

            Completed = true;
            frmUpdater.Finished = false;
        }

        static void _core_StatusChanged(object sender, StatusChangedEventArgs e)
        {
            if (e.StatusText != "Downloading")
            {
                return;
            }

            if (!Completed)
            {
                openUpdater = true;
                Completed = true;
            }
        }

        static void CheckCaches(string name)
        {
            if (!File.Exists(name))
            {
                return;
            }

            try
            {
                var xmlFile = XDocument.Load(name);
            }
            catch (Exception)
            {
                File.Delete(name);
            }
        }
    }

    public struct PROCESS_INFORMATION
    {
        public IntPtr hProcess;
        public IntPtr hThread;
        public uint dwProcessId;
        public uint dwThreadId;
    }

    public struct STARTUPINFO
    {
        public uint cb;
        public string lpReserved;
        public string lpDesktop;
        public string lpTitle;
        public uint dwX;
        public uint dwY;
        public uint dwXSize;
        public uint dwYSize;
        public uint dwXCountChars;
        public uint dwYCountChars;
        public uint dwFillAttribute;
        public uint dwFlags;
        public short wShowWindow;
        public short cbReserved2;
        public IntPtr lpReserved2;
        public IntPtr hStdInput;
        public IntPtr hStdOutput;
        public IntPtr hStdError;
    }

    public struct SECURITY_ATTRIBUTES
    {
        public int length;
        public IntPtr lpSecurityDescriptor;
        public bool bInheritHandle;
    }
}