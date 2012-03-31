using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using DBNUpdater;

namespace CitizenLauncher
{
    public partial class frmUpdater : Form
    {
        private Core _core;
        public static bool Finished { get; set; }

        public frmUpdater(Core core)
        {
            InitializeComponent();

            _core = core;

            timer1.Enabled = true;
            timer1.Interval = 150;
            timer1.Tick += new EventHandler(timer1_Tick);
            //label1.Text = "Initializing...";
        }

        void timer1_Tick(object sender, EventArgs e)
        {
            if (Finished)
            {
                Close();
            }

            /*timer1.Enabled = false;

            Visible = false;
            Hide();

            InitializeUpdater();*/
        }

        private void frmUpdater_Load(object sender, EventArgs e)
        {
            InitializeUpdater();
        }

        private void InitializeUpdater()
        {
            _core.Completed += new EventHandler<EventArgs>(_core_Completed);
            //_core.Failed += new EventHandler<FailedEventArgs>(_core_Failed);
            _core.StatusChanged += new EventHandler<StatusChangedEventArgs>(_core_StatusChanged);
        }

        void _core_StatusChanged(object sender, StatusChangedEventArgs e)
        {
            try
            {
                this.Invoke(new Action<StatusChangedEventArgs>(ev =>
                {
                    /*if (ev.StatusText == "Downloading")
                    {
                        this.Visible = true;
                        this.Show();
                    }*/

                    this.label1.Text = string.Format("{0} {1}%", ev.DetailedStatus, ev.ExactPercentage);
                    this.progressBar1.Minimum = 0;
                    this.progressBar1.Maximum = (int)ev.TotalTasks;
                    this.progressBar1.Value = (int)ev.TasksDone;
                }), e);
            }
            catch { }
        }

        void _core_Completed(object sender, EventArgs e)
        {
            try
            {
                this.Invoke(new Action(() =>
                {
                    Finished = true;
                    this.Close();
                }));
            }
            catch { }
        }

        void _core_Failed(object sender, FailedEventArgs e)
        {
            try
            {
                MessageBox.Show(e.Message, "alterIWnate", MessageBoxButtons.OK, MessageBoxIcon.Error);

                this.Invoke(new Action<FailedEventArgs>(ev =>
                {
                    Finished = false;
                    //this.label1.Text = ev.Message;
                }), e);
            }
            catch { }
        }
    }
}
