package swmem.org.sltran;

/**
 * Created by Gabriel on 2016. 2. 11..
 */
public class GRTClass {
    static {
        System.loadLibrary("GRT");
    }
    public native String getHelloNDKString();
    public native void loadModule();
}
