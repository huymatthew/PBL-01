#include <map>
#include <string>
using namespace std;
// ERROR: 5xxx
// SUCCESS: 6xxx
// WARNING: 7xxx
map<int, string> vie = {
    {1001, "DUTBANK xin chào"},
    {1002, "Nhập mã PIN:"},
    {1003, "Nhập mật khẩu mới:"},
    {1004, "Nhập lại mật khẩu mới:"},
    {1005, "Nhập số tiền:"},
    {1006, "Số tiền chuyển:"},
    {1007, "Chọn ngân hàng:"},
    {1008, "Chuyển đến:"},
    {1010, "Rút tiền"},
    {1020, "Chuyển tiền"},
    {1030, "Xem số dư"},
    {1040, "Đổi mã PIN"},
    {1050, "Lich sử giao dịch"},
    {5000, "Thoát"},
    {5001, "Đăng xuất"},
    {5005, "Rút thẻ"},
    {5100, "Sai mã PIN"},
    {5101, "Mã PIN không khớp"},
    {5102, "Tài khoản không tồn tại"},
    {6000, "Đồng ý"},
    {6001, "Thành công"},
    {6002, "Đăng nhập"},
    {6003, "Chuyển"},
    {7000, "Xóa"},
    {7001, "Khác"}
};

map<int, string> eng = {
    {1001, "Welcome to DUTBANK"},
    {1002, "Enter your PIN:"},
    {1003, "Enter new password:"},
    {1004, "Re-enter new password:"},
    {1005, "Enter amount:"},
    {1006, "Amount to transfer:"},
    {1007, "Select bank:"},
    {1008, "Transfer to:"},
    {1010, "Withdraw"},
    {1020, "Transfer"},
    {1030, "Check balance"},
    {1040, "Change PIN"},
    {1050, "Transaction history"},
    {5000, "Exit"},
    {5001, "Logout"},
    {5005, "Withdraw card"},
    {5100, "Wrong PIN"},
    {5101, "PIN mismatch"},
    {5102, "Account not found"},
    {6000, "OK"},
    {6001, "Success"},
    {6002, "Login"},
    {6003, "Transfer"},
    {7000, "Backspace"},
    {7001, "Other"}
};  

map<int, string> spa = {
    {1001, "Bienvenido a DUTBANK"},
    {1002, "Ingrese su PIN:"},
    {1003, "Ingrese nueva contraseña:"},
    {1004, "Vuelva a ingresar la nueva contraseña:"},
    {1005, "Ingrese la cantidad:"},
    {1006, "Cantidad a transferir:"},
    {1007, "Seleccione el banco:"},
    {1008, "Transferir a:"},
    {1010, "Retirar"},
    {1020, "Transferir"},
    {1030, "Consultar saldo"},
    {1040, "Cambiar PIN"},
    {1050, "Historial de transacciones"},
    {5000, "Salir"},
    {5001, "Cerrar sesión"},
    {5005, "Retirar tarjeta"},
    {5100, "PIN incorrecto"},
    {5101, "PIN no coincide"},
    {6000, "Aceptar"},
    {6001, "Éxito"},
    {6002, "Iniciar sesión"},
    {6003, "Transferir"},
    {7000, "Borrar"},
    {7001, "Otro"}
};
map<int, string> staticMap = {
    {1, "English"},
    {2, "Tiếng Việt"},
    {3, "Español"},
    {10, "Load Data"},
    {11, "Save Data"},
    {12, "Start"},
    {100, "100.000VND"},
    {101, "200.000VND"},
    {102, "500.000VND"},
    {103, "1.000.000VND"},
    {104, "2.000.000VND"},
    {105, "5.000.000VND"},
    {500, "MBBANK"},
    {501, "VIETCOMBANK"},
    {502, "VIETINBANK"},
    {503, "AGRIBANK"},
    {504, "BIDV"},
    {505, "SACOMBANK"},
    {506, "ACB"},
    {507, "VPBANK"},
    {508, "TECHCOMBANK"},
    {509, "TPBANK"},
    {510, "VIB"},
    {511, "OCB"},
    {512, "EXIMBANK"},
    {513, "MSBANK"},
    {514, "NAMABANK"},
    {515, "HDBANK"},
    {516, "PVCOMBANK"},
    {517, "SCB"},
    {518, "LienVietPostBank"},
    {519, "BACABANK"},
    {520, "BVB"},
    {521, "GPBANK"},
    {522, "SHB"},
    {523, "DongA"},
};
map<int, string> getLanguage(int lang){
    map<int, string> mergedMap = staticMap;
    switch (lang)
    {
    case 2:
        mergedMap.insert(vie.begin(), vie.end());
    case 3:
        mergedMap.insert(spa.begin(), spa.end());
        break;
    default:
        mergedMap.insert(eng.begin(), eng.end());
        break;
    }
    return mergedMap;
};