package swmem.org.sltran;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;

/**
 * Created by Gabriel on 2016. 3. 23..
 */
public class SocketManager {

    public final static String HOST = "211.189.127.61"; //real
    //public final static String HOST = "115.145.248.185"; //fake
    public final static int PORT = 4388;

    private static Socket socket;

    public static Socket getSocket() throws IOException
    {
        if( socket == null)
            socket = new Socket();

        if(!socket.isConnected())
            socket.connect(new InetSocketAddress(HOST, PORT));

        return socket;
    }

    public static void closeSocket() throws IOException
    {
        if ( socket != null )
            socket.close();
    }

    public static boolean isClosed(){
        if(socket == null){
            return true;
        }

        if (socket.isClosed()) {
            return true;
        } else {
            return false;
        }
    }

    public static void clearSocket(){
        socket = null;
    }

    public static void sendMsg(String msg) throws IOException
    {
        getSocket().getOutputStream().write((msg).getBytes());
    }

    public static void sendTxt(String path) throws IOException{
        FileTxThread fileTxThread = new FileTxThread(getSocket(),path);

        fileTxThread.start();
    }
}