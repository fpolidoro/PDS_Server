using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

enum UpdateType { WND_CREATED = 0, WND_FOCUSED = 1, WND_DESTROYED = 2 };
enum KeyCombTarget { FOCUSED_WND = 0, SPECIFIC_WND = 1 };

namespace ClientTest {
    class UpdateMessage {
        public UpdateType type;
        public int wndId;
        public string wndName;
        public string wndIcon;
    }
}
