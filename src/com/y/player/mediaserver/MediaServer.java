package com.y.player.mediaserver;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;

import org.teleal.cling.binding.annotations.AnnotationLocalServiceBinder;
import org.teleal.cling.model.DefaultServiceManager;
import org.teleal.cling.model.ValidationException;
import org.teleal.cling.model.meta.DeviceDetails;
import org.teleal.cling.model.meta.DeviceIdentity;
import org.teleal.cling.model.meta.LocalDevice;
import org.teleal.cling.model.meta.LocalService;
import org.teleal.cling.model.meta.ManufacturerDetails;
import org.teleal.cling.model.meta.ModelDetails;
import org.teleal.cling.model.types.DeviceType;
import org.teleal.cling.model.types.UDADeviceType;
import org.teleal.cling.model.types.UDN;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

public class MediaServer {

	private UDN udn = UDN.uniqueSystemIdentifier("GNaP-MediaServer");
	private LocalDevice localDevice;

	private final static String deviceType = "MediaServer";
	private final static int version = 1;
	private final static String LOGTAG = "GNaP-MediaServer";
	private final static int port = 8192;
	private static InetAddress localAddress;

	public MediaServer(InetAddress localAddress) throws ValidationException {
		DeviceType type = new UDADeviceType(deviceType, version);

		DeviceDetails details = new DeviceDetails(android.os.Build.MODEL,
				new ManufacturerDetails(android.os.Build.MANUFACTURER),
				new ModelDetails("GNaP", "GNaP MediaServer for Android", "v1"));

		LocalService service = new AnnotationLocalServiceBinder()
				.read(ContentDirectoryService.class);

		service.setManager(new DefaultServiceManager<ContentDirectoryService>(
				service, ContentDirectoryService.class));

		localDevice = new LocalDevice(new DeviceIdentity(udn), type, details,
				service);
		this.localAddress = localAddress;

		Log.v(LOGTAG, "MediaServer device created: ");
		Log.v(LOGTAG, "friendly name: " + details.getFriendlyName());
		Log.v(LOGTAG, "manufacturer: "
				+ details.getManufacturerDetails().getManufacturer());
		Log.v(LOGTAG, "model: " + details.getModelDetails().getModelName());
		
		//start http server
		try {
			new HttpServer(port);
		}
		catch (IOException ioe )
		{
			System.err.println( "Couldn't start server:\n" + ioe );
			System.exit( -1 );
		}
		
		Log.v(LOGTAG, "Started Http Server on port " + port);
	}

	public LocalDevice getDevice() {
		return localDevice;
	}

	public String getAddress() {
		return localAddress.getHostAddress() + ":" + port;
	}
}
