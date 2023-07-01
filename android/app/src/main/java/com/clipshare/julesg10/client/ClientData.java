package com.clipshare.julesg10.client;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class ClientData {
    public String text = "";
    public List<String> files = new ArrayList<>();
    public List<String> request_files = new ArrayList<>();
    public List<List<String>> response_files = new ArrayList<>();
    public int id = 0;

    public ClientData()
    {

    }

    private List<String> toList(JSONArray array) throws JSONException {
        List<String> list = new ArrayList<>();
        for (int i = 0; i < array.length(); i++) {
            String value = array.getString(i);
            list.add(value);
        }

        return list;
    }

    private List<List<String>> toListList(JSONArray array) throws JSONException {
        List<List<String>> list = new ArrayList<>();
        for (int i = 0; i < array.length(); i++) {
            List<String> inlist = this.toList(array.getJSONArray(i));
            list.add(inlist);
        }

        return list;
    }

    public static ClientData parse(String data) {
        ClientData clientData = new ClientData();
        try {
            JSONObject root = new JSONObject(data);

            clientData.id = root.getInt("id");
            clientData.request_files = clientData.toList(root.getJSONArray("request_files"));
            clientData.response_files = clientData.toListList(root.getJSONArray("response_files"));

            JSONObject clipboard = root.getJSONObject("clipboard");

            clientData.files = clientData.toList(clipboard.getJSONArray("files"));
            clientData.text = clipboard.getString("text");

        } catch (JSONException ignore) {}

        return clientData;
    }

    public String build()
    {
        JSONObject root = new JSONObject();
        JSONObject clipboard = new JSONObject();

        try {
            clipboard.put("text", text);
            clipboard.put("files", files);
            root.put("clipboard", clipboard);
            root.put("request_files",new ArrayList<String>());
            root.put("response_files",new ArrayList<String>());
            root.put("id", 0);
        } catch (JSONException e) {
            return "";
        }

        return root.toString();
    }
}
