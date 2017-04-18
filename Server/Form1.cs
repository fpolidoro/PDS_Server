using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using System.Net;
using System.Net.Sockets;

namespace ClientTest {
    public partial class Form1 : Form {

        int currentID, keyCode = 0;
        string keyString;
        bool alt = false, ctrl = false, shift = false;

        const string DEFAULT_SERVER = "localhost";
        const int DEFAULT_PORT = 27015;
        System.Net.Sockets.Socket socket;

        public Form1() {
            InitializeComponent();
        }

        public string Receive() {
            byte[] msglength = new byte[4];
            int read = socket.Receive(msglength);
            Int32 length = BitConverter.ToInt32(msglength, 0);

            byte[] message = new byte[length];
            socket.Receive(message);
            System.Text.Encoding encoding = System.Text.Encoding.ASCII;
            string stream = encoding.GetString(message);

            return stream;
        }

        public bool Send(string str) {
            byte[] message = System.Text.Encoding.ASCII.GetBytes(str + "\0");
            return (socket.Send(message)) > 0 ? true : false;
        }

        private void btnConnect_Click(object sender, EventArgs e) {
            try {
                IPHostEntry hostInfo = Dns.GetHostByName(DEFAULT_SERVER);
                IPAddress clientAddr = hostInfo.AddressList[0];
                var clientEndPoint = new IPEndPoint(clientAddr, DEFAULT_PORT);

                socket = new System.Net.Sockets.Socket(System.Net.Sockets.AddressFamily.InterNetwork,
                System.Net.Sockets.SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp);
                socket.Connect(clientEndPoint);
                btnConnect.Text = "Connected";
                btnConnect.Enabled = false;
            }
            catch (Exception ex) {
                Console.WriteLine(ex.Message);
                socket.Close();
                return;
            }
            return;
        }

        private void btnReceive_Click(object sender, EventArgs e) {
            string stream = Receive();
            UpdateMessage message = JsonConvert.DeserializeObject<UpdateMessage>(stream);

            currentID = message.wndId;
            lblID.Text = "" + message.wndId;
            lblTitle.Text = message.wndName;
            lblType.Text = message.type.ToString();

            try {
                imgIcon.Image = Base64ToImage(message.wndIcon);
            }
            catch { }
        }

        private static Image Base64ToImage(string base64String) {

            byte[] imageBytes = Convert.FromBase64String(base64String);
            Icon icon = new Icon(new MemoryStream(imageBytes));

            return icon.ToBitmap();
        }

        private void btnSend_Click(object sender, EventArgs e) {
            KeyMessage message = new KeyMessage();

            if (ctrl) message.keys[message.nKeys++] = (int)Keys.ControlKey;
            if (alt) message.keys[message.nKeys++] = (int)Keys.Alt;
            if (shift) message.keys[message.nKeys++] = (int)Keys.ShiftKey;
            if (keyCode != 0) message.keys[message.nKeys++] = keyCode;

            string str = JsonConvert.SerializeObject(message);
            Send(str);
        }

        private void txtComb_KeyDown(object sender, KeyEventArgs e) {
            if (e.Control)
                ctrl = !ctrl;
            else if (e.Alt)
                alt = !alt;
            else if (e.Shift)
                shift = !shift;

            switch (e.KeyCode) {
                case Keys.ControlKey:
                case Keys.Alt:
                case Keys.ShiftKey:
                case Keys.Menu:
                    break;
                default:
                    keyString = (e.KeyCode.ToString().Replace("Oem", string.Empty));
                    keyCode = (int)e.KeyCode;
                    break;
            }

            e.Handled = true;

            List<string> keys = new List<string>();
            if (ctrl) keys.Add("CTRL");
            if (alt) keys.Add("ALT");
            if (shift) keys.Add("SHIFT");
            keys.Add(keyString);
            txtComb.Text = string.Join(" + ", keys);
        }

        //private void btnProva2_Click(object sender, EventArgs e) {
        //    KeyMessage message = new KeyMessage();
        //    message.nKeys = 2;
        //    message.keys[0] = (int)Keys.ControlKey;
        //    message.keys[1] = (int)Keys.X;
        //    string str = JsonConvert.SerializeObject(message);
        //    Send(str);
        //}
    }
}
