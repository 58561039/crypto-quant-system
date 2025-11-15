/*
write by lzc
Email: 544840897@qq.com


三个websocket的回调函数的声明
*/


#pragma once

#include <string>

void public_websocket_callback(const std::string& msg);
void orders_websocket_callback(const std::string& msg);
void orders_algo_websocket_callback(const std::string& msg);
