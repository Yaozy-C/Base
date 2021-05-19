//
// Created by Yaozy on 2019/10/9.
//
#include "DataPacket.h"
#include "Log.h"

namespace Base {

    void GetJsonValue(cJSON *node, string &value) {
        if (!cJSON_IsNull(node) && (cJSON_IsString(node) || cJSON_IsNumber(node)))
            value = node->valuestring;
        else
            value = "";
    }

    void GetJsonValue(cJSON *node, int &value) {
        if (!cJSON_IsNull(node) && cJSON_IsNumber(node))
            value = node->valueint;
        else
            value = 0;
    }

    void GetJsonValue(cJSON *node, double &value) {
        if (!cJSON_IsNull(node) && cJSON_IsNumber(node))
            value = node->valuedouble;
        else
            value = 0.0;
    }

    void GetJsonValue(cJSON *node, bool &value) {
        value = cJSON_IsTrue(node) != 0;
    }

    void GetJsonValue(cJSON *node, DataPacket &value) {
        value.DecodeJson(node);
    }

    cJSON *cJSON_Create(string &value) {
        return cJSON_CreateString(value.c_str());
    }

    cJSON *cJSON_Create(int &value) {
        return cJSON_CreateNumber(value);
    }

    cJSON *cJSON_Create(double &value) {
        return cJSON_CreateNumber(value);
    }

    cJSON *cJSON_Create(float &value) {
        return cJSON_CreateNumber(value);
    }

    cJSON *cJSON_Create(bool &value) {
        return cJSON_CreateBool(value);
    }

    cJSON *cJSON_Create(DataPacket &value) {
        cJSON *item = value.CreateWriter();
        value.EncodeJson(item);
        return item;
    }

    string DataPacket::GetJsonText() {
        cJSON *writer = CreateWriter();
        try {
            EncodeJson(writer);
        } catch (...) {
            LOG_ERROR("Encode json error");
        }
        char *p = cJSON_PrintUnformatted(writer);
        string json = p;
        cJSON_Delete(writer);
        free(p);
        return json;
    }

    void DataPacket::SetJsonText(const string &text) {
        cJSON *reader = cJSON_Parse(text.c_str());
        if (reader != nullptr) {
            try {
                DecodeJson(reader);
            } catch (...) {
                LOG_ERROR("Decode json error");
            }
            cJSON_Delete(reader);
        } else {
            LOG_ERROR("This text isn't a json string!");
        }
    }

    cJSON *DataPacket::CreateWriter() {
        return cJSON_CreateObject();
    }
}