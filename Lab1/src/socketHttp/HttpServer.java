package socketHttp;

import javax.annotation.processing.Filer;
import javax.print.attribute.standard.PrinterResolution;
import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Path;
import java.sql.Time;
import java.sql.Timestamp;
import java.time.LocalDateTime;
import java.time.LocalTime;

public class HttpServer extends Thread {
    int port;
    ServerSocket serverSocket = null;
    String mainDirectory = null;


    public HttpServer(int port, String mainDirectory) {
        this.port = port;
        this.mainDirectory = mainDirectory;
        try {
            serverSocket = new ServerSocket(port);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    public void setMainDirectory(String mainDirectory) {
        this.mainDirectory = mainDirectory;
    }

    public void setPort(int port) {
        this.port = port;
    }

    @SuppressWarnings("InfiniteLoopStatement")
    public void run() {
        //首部分块数组
        while (true) {
            System.out.println("服务器等待外部连接中……\n");
            try {
                //监听端口，建立TCP连接
                Socket socket = serverSocket.accept();
                //获取socket开始连接的时间
                Timestamp startTime = Timestamp.valueOf(LocalDateTime.now());
                //超时标志
                boolean timeOut = false;
                System.out.println("已与主机" + socket.getInetAddress() + ":"
                        + socket.getPort() + "建立TCP连接。\n");
                //实例化输入输出流对象，读取http请求报文并返回响应报文
                BufferedReader bufferedReader =
                        new BufferedReader(new InputStreamReader(socket.getInputStream()));
                String line = null;
                StringBuilder request = new StringBuilder();
                //反复读取，直到读取到请求报文
                do {
                    if (Timestamp.valueOf(LocalDateTime.now()).getTime() - startTime.getTime() > 10000) {
                        timeOut = true;
                        break;
                    }
                    line = bufferedReader.readLine();
                } while (line == null || line.equals(""));
                //超时检查
                if (timeOut) {
                    System.out.println("连接超时，超时期间未收到请求报文\n");
                } else {
                    //请求报文首行
                    String firstLine = line;
                    //读取请求报文全部内容
                    request.append(line).append("\r\n");
                    while ((line = bufferedReader.readLine()) != null && !line.equals("")) {
                        request.append(line).append("\r\n");
                    }
                    System.out.println("HTTP请求报文 ------------------------\n" + request.toString()
                            + "-------------------------------------\n");
                    //首行分块数组
                    String[] chunks = firstLine.split(" ");
                    //确认http版本
                    if (chunks.length != 3 || !chunks[2].equals("HTTP/1.1")) {
                        System.out.println("会话: " + socket.toString() + " 结束\n");
                        socket.close();
                        break;
                    }
                    //只处理GET请求
                    if (chunks[0].equals("GET")) {
                        //输出流
                        OutputStream outputStream = socket.getOutputStream();
                        OutputStreamWriter outputStreamWriter = new OutputStreamWriter(outputStream);
                        PrintWriter printWriter = new PrintWriter(outputStreamWriter);
                        File file = null;
                        if (chunks[1].equals("/")) {
                            //重定向到主页面
                            file = new File(mainDirectory + "/index.html");
                        } else {
                            file = new File(mainDirectory + chunks[1]);
                        }
                        //查找文件是否存在
                        if (file.isFile()) {
                            FileInputStream fileInputStream = new FileInputStream(file);
                            //首部
                            printWriter.println("HTTP/1.1 200 OK");
                            StringBuilder typeMsg = new StringBuilder("Content-Type: ");
                            String filename = file.getName();
                            //文件扩展名
                            String ext = filename.substring(filename.lastIndexOf(".") + 1);
                            if (ext.equals("html")) {
                                typeMsg.append("text/html; charset=UTF-8");
                            } else if (ext.equals("jpg")) {
                                typeMsg.append("image/jpeg");
                            } else if (ext.equals("ico")) {
                                typeMsg.append("image/x-icon");
                            }
                            printWriter.println(typeMsg.toString());
                            printWriter.print("Content-Length: ");
                            printWriter.println(file.length());
                            printWriter.println("");
                            printWriter.flush();
                            //主体
                            byte[] buf = new byte[2048];
                            while (fileInputStream.read(buf, 0, buf.length) != -1) {
                                outputStream.write(buf, 0, buf.length);
                            }
                            System.out.println("已发送文件" + file.getName() + "\n");
                            fileInputStream.close();
                            outputStream.flush();
                            outputStream.close();
                        } else {
                            //文件不存在，返回404页面
                            printWriter.println("HTTP/1.1 404 NOT FOUND");
                            //复制404页面主体到缓冲区
                            file = new File(mainDirectory + "/404.html");
                            BufferedReader fileBuffer = new BufferedReader(new FileReader(file));
                            StringBuilder content = new StringBuilder();
                            while ((line = fileBuffer.readLine()) != null) {
                                content.append(line).append("\r\n");
                            }
                            printWriter.print("Content-Length: ");
                            printWriter.println(file.length());
                            printWriter.println("Content-Type: text/html; charset=UTF-8");
                            printWriter.println("");
                            printWriter.print(content.toString());
                            printWriter.flush();
                            printWriter.close();
                            fileBuffer.close();
                            System.out.println("404 请求资源不存在\n");
                        }
                    }
                }
                System.out.println("会话 " + socket.toString() + " 结束\n");
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
