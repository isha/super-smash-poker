package com.example.supersmashpoker;

import java.net.Socket;
import android.app.Application;

public class SuperSmashPoker extends Application {
	Socket socket = null;
	String ip = "192.168.1.103";
	int port = 50002;
	byte client_id = 0;
}
