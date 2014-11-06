/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hybroad.json;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.text.TextUtils;
import android.util.Log;

/**
 * 
 * @author WTZ and ZJF
 * @since 2013-05-20
 * 
 */
public class JSONUtils {
    private final static String TAG = "---JSONUtils---";

    /**
     * 
     * It's used to parse JSON and set data to givien Object.
     * 
     * @param jsonString
     *            JSON-encoded string containing an object. The value of member
     *            in this object is String or JSON-Array.
     * @param object
     *            This object is used to receive data parsed from JSON,
     *            attribute type of the object is between String, int,
     *            ArrayList. There must be a no-argument constructor, setters
     *            and getters in the class of object. The setters and getters
     *            should be named by the default standard, e.g. the setter of
     *            attribute 'count' should be named as 'setCount'.
     * @return The input object which is set value.
     */
    public static Object loadObjectFromJSON(String jsonString, Object object) {
        if (object == null || TextUtils.isEmpty(jsonString)) {
            return null;
        }

        try {
            JSONObject jsonObject = new JSONObject(jsonString);
            Iterator keys = jsonObject.keys();
            while (keys.hasNext()) {
                try {
                    String key = keys.next().toString();
                    String value = jsonObject.getString(key);
                    // Log.d(TAG, "----key: " + key + ", value: " + value);

                    setFieldByKey(object, key, value);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                } catch (NoSuchFieldException e) {
                    e.printStackTrace();
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return object;
    }

    private static void setFieldByKey(Object parentObject, String key, String value) throws IllegalAccessException,
            InvocationTargetException, JSONException, NoSuchFieldException {
        if (parentObject == null || TextUtils.isEmpty(key) || TextUtils.isEmpty(value)) {
            return;
        }

        char head = key.charAt(0);
        String tail = key.length() > 1 ? key.substring(1) : null;
        String currentMethod = "set" + Character.toUpperCase(head) + tail;

        Class clazz = parentObject.getClass();
        Method[] methods = clazz.getDeclaredMethods();
        for (Method method : methods) {
            String methodName = method.getName();
            if (currentMethod.equals(methodName)) {
                Type[] parameterTypes = method.getGenericParameterTypes();
                if (parameterTypes == null || parameterTypes.length < 1) {
                    return;
                }

                Type parameterType = parameterTypes[0];
                if (parameterType == null) {
                    return;
                }
                
                // Log.d(TAG, "parameterType: " + parameterType.toString() + "===================================");
                if (String.class.equals(parameterType)) {
                    method.invoke(parentObject, value);
                } else if (parameterType.toString().equals("int")) {
                    if (isInteger(value)) {
                        method.invoke(parentObject, Integer.parseInt(value));
                    }
                } else if (value.startsWith("[") && value.endsWith("]")) { // JSON-Array
                    JSONArray jsonArray = new JSONArray(value);
                    if (jsonArray == null || jsonArray.length() <= 0) {
                        return;
                    }

                    try {
                        // 'ArrayList < Class Name >'
                        String tempString = parameterType.toString();
                        int start = tempString.indexOf('<');
                        int end = tempString.indexOf('>');
                        if (start < 0 || end < 0) {
                            break;
                        }

                        String className = tempString.substring(start + 1, end);
                        Class arrayMemberClass = Class.forName(className);
                        ArrayList<Object> arrayList = new ArrayList<Object>();
                        int size = jsonArray.length();
                        for (int i = 0; i < size; i++) {
                            Object subobject;
                            try {
                                subobject = arrayMemberClass.newInstance();
                            } catch (InstantiationException e) {
                                e.printStackTrace();
                                continue;
                            }
                            Object jsonObj = jsonArray.get(i);
                            if (jsonObj == null || !(jsonObj instanceof JSONObject)) {
                                continue;
                            }
                            subobject = loadObjectFromJSON(jsonObj.toString(), subobject); // Recursive
                            arrayList.add(subobject);
                        }
                        method.invoke(parentObject, arrayList);
                    } catch (ClassNotFoundException e) {
                        e.printStackTrace();
                    }

                } else if (value.startsWith("{") && value.endsWith("}")) { // JSON-Object
                    try {
                        // JSON-Object Class Name
                        String tempString = parameterType.toString();
                        if (!tempString.startsWith("class ")) {
                            break;
                        }
                        if (tempString.length() < 7) {
                            break;
                        }
                        
                        String className = tempString.substring(6);
                        Class jsonObjectClass = Class.forName(className);
                        Object subobject;
                        try {
                            subobject = jsonObjectClass.newInstance();
                            subobject = loadObjectFromJSON(value, subobject); // Recursive
                            method.invoke(parentObject, subobject);
                        } catch (InstantiationException e) {
                            e.printStackTrace();
                        }
                    } catch (ClassNotFoundException e) {
                        e.printStackTrace();
                    }
                } 
                // exit this loop after handler
                break;
            }
        }
    }

    /**
     * 
     * @param str
     *            the string to be examined.
     * @return true if str is Integer or not.
     */
    public static boolean isInteger(String str) {
        Pattern pattern = Pattern.compile("^[-\\+]?[\\d]+$");
        Matcher matcher = pattern.matcher((CharSequence) str);
        return matcher.find();
    }
}
