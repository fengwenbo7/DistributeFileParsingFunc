/*
fengwenbo 2022-06
*/

#include <unistd.h>
#include "server/webserver.h"

int main(){
        WebServer server(
        1316, TrigMode::kAllET, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "root", "webserver", /* Mysql配置 */
        12, 6);             /* 连接池数量 线程池数量 */
    server.Start();
}