package swmem.org.sltran;

import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

/**
 * Created by Gabriel on 2016. 3. 23..
 */
public class FileTxThread extends Thread {
    Socket socket;
    String filePath;

    FileTxThread(Socket socket, String path){
        this.socket= socket;
        this.filePath = path;
    }

    @Override
    public void run() {
        File file = new File(this.filePath);

        byte[] bytes = new byte[(int) file.length()];
        BufferedInputStream bis;
        try {
            BufferedReader in = new BufferedReader(new FileReader(file));
            String s;

            OutputStream os = socket.getOutputStream();
            while ((s = in.readLine()) != null) {
                int len = s.getBytes().length;
                String lenStr = String.format("%03d",len);
                String ts = lenStr + " " + s;
                os.write(ts.getBytes(), 0, ts.getBytes().length);
            }
            String eof = "\u001a";
            os.write(eof.getBytes(),0,eof.length());
            in.close();
            /*
            bis = new BufferedInputStream(new FileInputStream(file));
            bis.read(bytes, 0, bytes.length);*/
            os.flush();


            String sentMsg = "File sent to: " + socket.getInetAddress();
            Log.v("FileSending",sentMsg);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } finally {
            /*
            try {

            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            */
        }

    }
}
