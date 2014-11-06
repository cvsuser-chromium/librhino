package com.hybroad.video;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import android.annotation.SuppressLint;
import android.util.Log;

/**
 * the class of subtile info CNcomment:get the sub-title info save it
 * 
 * @author xiongCuiFan
 * 
 */
public class SubXmlParser {
  /* the path of subtitle file */
  private String TAG = "SubXmlParser";
  private String path = "/etc/subtitle_cfg/style.xml";
  // private String dataPath = "/data/data/com.huawei.activity/style.xml";
  @SuppressLint("SdCardPath")
  private String dataPath = "/data/data/com.hybroad.media/style.xml";

  /**
   * Get the subtitle setting info by parse the XML file.
   * 
   * @return List
   */
  public List<Map<String, String>> subXmlParser_getValueList() {
    NamedNodeMap fontItem = xmlParser();
    if (fontItem == null) {
      return null;
    }
    List<Map<String, String>> pointList = new ArrayList<Map<String, String>>();
    Map<String, String> pointMap;
    Node pointNode;
    for (int i = 0; i < fontItem.getLength(); i++) {
      pointNode = fontItem.item(i);
      pointMap = new HashMap<String, String>();
      pointMap.put(pointNode.getNodeName(), pointNode.getNodeValue());
      pointList.add(pointMap);
    }
    return pointList;
  }

  /**
   * Get the subtitle setting info by parse the XML file.
   * 
   * @return Object Array
   */
  public Object[] subXmlParser_getValueArray() {
    NamedNodeMap fontItem = xmlParser();
    if (fontItem == null) {
      return null;
    }
    Object[] obj = new Object[fontItem.getLength()];
    for (int i = 0; i < fontItem.getLength(); i++) {
      obj[i] = fontItem.item(i).getNodeValue();
      Log.i("cuifan", "in parser,obj[" + i + "]=" + obj[i]);
    }
    return obj;
  }

  /**
   * save subtile setting info
   * 
   * @param list
   *          the list of subtitle info
   */
  public void subXmlParser_setValue(List<?> list) {

    DocumentBuilderFactory factory = null;
    DocumentBuilder builder = null;
    Document document = null;
    NamedNodeMap fontItem = null;
    Node pointNode = null;
    File file = new File(dataPath);
    if (file != null) {
      try {
        factory = DocumentBuilderFactory.newInstance();
        builder = factory.newDocumentBuilder();
        document = builder.parse(file);
      } catch (ParserConfigurationException e) {
        e.printStackTrace();
      } catch (SAXException e) {
        e.printStackTrace();
      } catch (IOException e) {
        e.printStackTrace();
      }
      if (document != null) {
        Node fontNode = document.getElementsByTagName("font").item(0);
        fontItem = fontNode.getAttributes();
      }
    }

    if (fontItem == null) {
      return;
    }

    for (int i = 1; i < fontItem.getLength(); i++) {
      pointNode = fontItem.item(i);
      String str = list.get(i).toString();
      if (!("-1".equals(str))) {
        pointNode.setNodeValue(str);
      }
    }

    TransformerFactory tf = TransformerFactory.newInstance();
    try {
      Transformer tr = tf.newTransformer();
      DOMSource source = new DOMSource(document);
      PrintWriter pw = new PrintWriter(new File(dataPath));
      StreamResult result = new StreamResult(pw);
      tr.transform(source, result);
      Log.i(TAG, "----------->sub save success");

    } catch (TransformerConfigurationException e) {
      e.printStackTrace();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (TransformerException e) {
      e.printStackTrace();
    }
  }

  /**
   * get the subtitle node
   * 
   * @return
   */
  public NamedNodeMap xmlParser() {
    File file = new File(dataPath);
    if (file != null) {
      DocumentBuilderFactory factory = null;
      DocumentBuilder builder = null;
      Document document = null;
      try {
        factory = DocumentBuilderFactory.newInstance();
        builder = factory.newDocumentBuilder();
        document = builder.parse(file);

      } catch (ParserConfigurationException e) {
        e.printStackTrace();
      } catch (SAXException e) {
        e.printStackTrace();
      } catch (IOException e) {
        e.printStackTrace();
      }
      if (document != null) {
        Node fontNode = document.getElementsByTagName("font").item(0);
        return (fontNode.getAttributes());
      }
    }
    return null;
  }

  /**
   * copy the XML file to the directory "/data"
   */
  public void copyXmlFile() {

    File etcFile = new File(path);
    File dataFile = new File(dataPath);
    if (dataFile.exists()) {
      return;
    }
    try {
      InputStream in = new FileInputStream(etcFile);
      OutputStream out = new FileOutputStream(dataFile);

      byte[] buf = new byte[1024];
      int len;
      while ((len = in.read(buf)) > 0) {
        out.write(buf, 0, len);
      }
      Log.i(TAG, "-------->copy xml file success");
      in.close();
      out.close();
      Process process = Runtime.getRuntime().exec("chmod 777 " + dataPath);
      int status = process.waitFor();
      if (status == 0) {
        Log.i(TAG, "chmod success");
      }

    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

}
