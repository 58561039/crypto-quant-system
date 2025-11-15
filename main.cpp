
/*
write by lzc
Email: 544840897@qq.com
*/


#include"quant.h"


int main() {

    //使用之前，先填写apiKey，secretKey，passphrase
    //实盘就写实盘的API，模拟盘就写模拟盘的API
    std::string apiKey     = "Your apiKey";
    std::string secretKey  = "Your secretKey";
    std::string passphrase = "Your passphrase";
    bool isdemo = true;//isdemo=true时连接okx的模拟盘，isdemo=false时连接okx的实盘


    auto& quant=Quant::GetInstance();

    quant.init(apiKey,secretKey,passphrase,isdemo);

    //目前仅支持永续合约，且默认是全仓
    //每个品种的订单记录都会输出到该品种对应的文件中，例如BTC-USDT-SWAP的交易记录输出到BTC-USDT-SWAP.csv中

    //添加交易品种   交易对名称  杠杆大小    可同时交易多个品种
    quant.add_symbol("BTC-USDT-SWAP",10);
    quant.add_symbol("ETH-USDT-SWAP",5);
    quant.add_symbol("SOL-USDT-SWAP",3);

    //连接交易所并订阅数据
    quant.connect_and_subscribe();

    //运行
    quant.run();




    //运行过程中随时输入quit结束运行，优雅退出
    std::string input;
    std::cout << "请输入命令 (输入 quit 退出程序):\n";

    while (true) {
        std::getline(std::cin, input); //读取整行输入

        if (input == "quit") {
            
            quant.stop();

            break; //跳出循环
        }
        std::cout << "你输入了: " << input << std::endl;
    }
}