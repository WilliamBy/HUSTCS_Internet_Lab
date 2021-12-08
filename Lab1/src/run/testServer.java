package run;

import socketHttp.HttpServer;

import java.io.File;
import java.nio.file.Path;
import java.util.Scanner;

public class testServer {
    public static void main(String[] args) {
        System.out.println("HTTP Server 测试系统\n\n请配置web主目录路径: ");
        String mainDir;
        Scanner scanner = new Scanner(System.in);
        while (true) {
            mainDir = scanner.next();
            File mainDirPath = new File(mainDir);
            if (mainDirPath.isDirectory()) {
                break;
            } else {
                System.out.println("目录不存在，请重新输入：");
            }
        }
        int port;
        System.out.println("请配置监听端口: ");
        port = scanner.nextInt();
        HttpServer server = new HttpServer(port, mainDir);
        server.start();
    }
}
