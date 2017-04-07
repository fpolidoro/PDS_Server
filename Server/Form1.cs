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
                //imgIcon.Image = Image.FromFile("C:\\Users\\Davide\\Desktop\\Progetto malnati\\ClientTest\\ClientTest\\default.ico");
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

            lblID.Text = "" + message.wndId;
            lblTitle.Text = message.wndName;
            lblType.Text = message.type.ToString();

            try {
                imgIcon.Image = Base64ToImage(message.wndIcon);
            }
            catch {}
        }

        private static Image Base64ToImage(string base64String) {

            byte[] imageBytes = Convert.FromBase64String(base64String);
            Icon icon = new Icon(new MemoryStream(imageBytes));

            return icon.ToBitmap();
        }
    }
}
