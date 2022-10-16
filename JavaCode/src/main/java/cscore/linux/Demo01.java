package cscore.linux;

public class Demo01 {

  public static void main(String[] args) {
    var thread = new Thread(new Runnable() {
      @Override
      public void run() {
        System.out.println("Hello world from a Java thread");
      }
    });
    thread.start();
  }
}
