using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

using Microsoft.Win32;

namespace DBNetwork.Citizen.Launcher
{
    public static class ConnectionManager
    {
        public static void Install()
        {
            if (Environment.OSVersion.Platform != PlatformID.Win32NT)
            {
                return;
            }

            try
            {
                var exePath = Assembly.GetExecutingAssembly().Location;

                var hkcu = Registry.CurrentUser;

                var key = hkcu.CreateSubKey(@"SOFTWARE\Classes\aiw");
                key.SetValue(null, "URL:aIW Protocol");
                key.SetValue("URL Protocol", "");

                var icon = key.CreateSubKey("DefaultIcon");
                icon.SetValue(null, exePath + ",1");
                icon.Close();

                var open = key.CreateSubKey(@"shell\open\command");
                open.SetValue(null, string.Format("\"{0}\" \"%1\"", exePath));
                open.Close();

                key.Close();
            }
            catch (Exception e)
            {
                MessageBox.Show("THIS ERROR IS NOT FATAL: " + e.ToString(), "alterIWnate", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public static void Handle(string[] args, bool isInitial)
        {
            var query = string.Join(" ", args);

            if (!query.StartsWith("aiw://"))
            {
                return;
            }

            var baseQuery = query.Replace("aiw://", "");
            var data = baseQuery.Split('/');

            switch (data[0])
            {
                case "connect":
                    // OMGZ CONNECT FEATURE IS NEARLY INVTIE SYSTEM
                    if (data.Length != 2)
                    {
                        goto default;
                    }

                    var ip = data[1];

                    PerformConnect(ip, isInitial);
                    break;
                default:
                    MessageBox.Show(string.Format("WHAT THE HELL ARE YOU THINKING? {0} DOESN'T EVEN EXIST, YOU NAB!", data[0].ToUpper()), "ANGRYIWNET");
                    break;
            }
        }

        private static void PerformConnect(string ip, bool isInitial)
        {
            var process = OpenIW4Process();

            if (process == IntPtr.Zero)
            {
                return;
            }

            _process = process;

            WaitForConsole();

            if (!isInitial)
            {
                WaitForLog("LSPXUID");
            }

            SendToInput("connect " + ip.Replace(" ", "").Replace(";", ""));
        }

        private static IntPtr OpenIW4Process()
        {
            var processes = Process.GetProcessesByName("iw4mp.dat");

            foreach (var process in processes)
            {
                return process.Handle;
            }

            return IntPtr.Zero;
        }

        private static void SendToInput(string text)
        {
            // send text first
            SendMessage(_hwndInput, WM_SETTEXT, IntPtr.Zero, text);

            // and send a \r
            SendMessage(_hwndInput, WM_CHAR, 0xD, 0);
        }

        private static void WaitForLog(string contains)
        {
            while (true)
            {
                Thread.Sleep(50);

                var content = GetLogContent();

                if (content.Contains(contains))
                {
                    break;
                }
            }
        }

        private static string GetLogContent()
        {
            return GetEditText(_hwndLog);
        }

        private static string GetEditText(IntPtr hwnd)
        {
            var len = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, null);

            var sb = new StringBuilder(len);
            SendMessage(hwnd, WM_GETTEXT, len + 1, sb);

            return sb.ToString();
        }

        private static void WaitForConsole()
        {
            while (true)
            {
                Thread.Sleep(1);

                ReadInputHwnd();
                ReadLogHwnd();

                if (_hwndInput != IntPtr.Zero && _hwndLog != IntPtr.Zero)
                {
                    break;
                }
            }
        }

        private static void ReadInputHwnd()
        {
            _hwndInput = ReadIntPtr(_hwndConsoleInpEditLoc);
        }

        private static void ReadLogHwnd()
        {
            _hwndLog = ReadIntPtr(_hwndConsoleLogEditLoc);
        }

        private static int ReadInt32(int location)
        {
            int bytes;
            byte[] buffer = new byte[4];
            ReadProcessMemory(_process, new IntPtr(location), buffer, 4, out bytes);

            return BitConverter.ToInt32(buffer, 0);
        }

        private static IntPtr ReadIntPtr(int location)
        {
            return new IntPtr(ReadInt32(location));
        }

        private static IntPtr _hwndInput;
        private static IntPtr _hwndLog;

        private static IntPtr _process;
        private static readonly int _hwndConsoleLogEditLoc = 0x64FEE9C;
        private static readonly int _hwndConsoleInpEditLoc = 0x64FEEA8;

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool ReadProcessMemory(
          IntPtr hProcess,
          IntPtr lpBaseAddress,
          [Out] byte[] lpBuffer,
          int dwSize,
          out int lpNumberOfBytesRead
         );

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static extern int SendMessage(IntPtr hWnd, UInt32 Msg, int wParam, StringBuilder lParam);

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static extern IntPtr SendMessage(IntPtr hWnd, UInt32 Msg, IntPtr wParam, StringBuilder lParam);

        [DllImport("user32.dll")]
        static extern IntPtr SendMessage(IntPtr hWnd, UInt32 Msg, IntPtr wParam, [MarshalAs(UnmanagedType.LPStr)] string lParam);

        [DllImport("user32.dll", EntryPoint = "SendMessageW")]
        static extern IntPtr SendMessageW(IntPtr hWnd, UInt32 Msg, IntPtr wParam, [MarshalAs(UnmanagedType.LPWStr)] string lParam);

        [DllImport("user32.dll")]
        static extern IntPtr SendMessage(IntPtr hWnd, UInt32 Msg, Int32 wParam, Int32 lParam);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = false)]
        static extern IntPtr SendMessage(HandleRef hWnd, UInt32 Msg, IntPtr wParam, IntPtr lParam);

        public const uint WM_SETTEXT = 0x0C;
        public const uint WM_GETTEXT = 0x0D;
        public const uint WM_GETTEXTLENGTH = 0x0E;
        public const uint WM_CHAR = 0x102;
    }
}
