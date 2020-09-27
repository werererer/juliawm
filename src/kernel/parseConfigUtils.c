#include "parseConfigUtils.h"
#include <stdlib.h>
#include <string.h>

static int jlArrLen(char *name) {
  jl_value_t *arr = jl_eval_string(name);
  return jl_array_len(arr);
}

void initConfig(char *path)
{
    char *prefix = "include(\"";
    char *postfix = "\")";
    //pluse one for the null char after string
    int charPtrlen = strlen(path) + strlen(prefix) + strlen(postfix) + 1;
    char res[charPtrlen];
    strcpy(res, prefix);
    strcat(res, path);
    strcat(res, postfix);
    jl_eval_string(res);
}

char* getConfigStr(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return (char *)jl_string_ptr(v);
}

float getConfigFloat(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return jl_unbox_float64(v);
}

int getConfigInt(char *name)
{
    jl_value_t* v = jl_eval_string(name);
    return jl_unbox_int32(v);
}

jl_function_t* getConfigFunc(char *name)
{
    return jl_eval_string(name);
}

Layout getConfigLayout(char *name)
{
    Layout layout;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    layout.symbol = getConfigStr(execStr1);
    layout.arrange = getConfigFunc(execStr2);
    return layout;
}

Hotkey getConfigHotkey(char *name)
{
    Hotkey hotkey;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);

    hotkey.symbol = getConfigStr(execStr1);
    hotkey.func = getConfigFunc(execStr2);
    return hotkey;
}

Rule getConfigRule(char *name)
{
    Rule rule;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];
    char execStr3[len];
    char execStr4[len];
    char execStr5[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);
    arrayPosToStr(execStr3, name, 3);
    arrayPosToStr(execStr4, name, 4);
    arrayPosToStr(execStr5, name, 5);

    rule.id = getConfigStr(execStr1);
    rule.isfloating = getConfigInt(execStr2);
    rule.monitor = getConfigInt(execStr3);
    rule.tags = getConfigInt(execStr4);
    rule.title = getConfigStr(execStr5);
    return rule;
}

MonitorRule getConfigMonRule(char *name)
{
    MonitorRule monrule;
    int len = ARR_STRING_LENGTH(name);
    char execStr1[len];
    char execStr2[len];
    char execStr3[len];
    char execStr4[len];
    char execStr5[len];
    char execStr6[len];

    arrayPosToStr(execStr1, name, 1);
    arrayPosToStr(execStr2, name, 2);
    arrayPosToStr(execStr3, name, 3);
    arrayPosToStr(execStr4, name, 4);
    arrayPosToStr(execStr5, name, 5);
    arrayPosToStr(execStr6, name, 6);

    monrule.name = getConfigStr(execStr1);
    monrule.mfact = getConfigFloat(execStr2);
    monrule.nmaster = getConfigInt(execStr3);
    monrule.scale = getConfigFloat(execStr4);
    getConfigLayoutArr(monrule.lt, execStr5);
    monrule.rr = getConfigInt(execStr6);
    return monrule;
}

void getConfigStrArr(char **resArr, char *name)
{
    char execStr[ARR_STRING_LENGTH(name)];
    int len = jlArrLen(name);

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigStr(execStr);
    }
}

void getConfigIntArr(int resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigInt(execStr);
    }
}

void getConfigFloatArr(float resArr[], char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        resArr[i] = getConfigFloat(execStr);
    }
}

void getConfigHotkeyArr(Hotkey *hotkeys, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        hotkeys[i] = getConfigHotkey(execStr);
    }
}

void getConfigRuleArr(Rule *rules, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];

    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        rules[i] = getConfigRule(execStr);
    }
}

void getConfigLayoutArr(Layout *layouts, char *name)
{
    int len = jlArrLen(name);
    char execStr[ARR_STRING_LENGTH(name)];
    char execStrFunc[ARR_STRING_LENGTH(name)];

    for (int i = 1; i <= len; i++) {
        arrayPosToStr(execStr, name, i);
        layouts[i-1] = getConfigLayout(execStr);
    }
}

void getConfigMonRuleArr(MonitorRule *monrules, char *name)
{
    int len = jlArrLen(name);
    jl_value_t *t;
    char execStr[ARR_STRING_LENGTH(name)];
    for (int i = 0; i < len; i++) {
        arrayPosToStr(execStr, name, i+1);
        monrules[i] = getConfigMonRule(execStr);
    }
}

//utils
/*
 * convert an integer to a char* with brackets.
 * Example 1 -> "[1]"
 * */
void appendIndex(char *resStr, int i)
{
    char d[5];
    sprintf(d, "%d", i);
    strcat(resStr, "[");
    strcat(resStr, d);
    strcat(resStr, "]");
}

/* 
 * convert two integers to a char* with brackets.
 * Example 1 2 -> "[1][2]"
 * */
static void append2DIndex(char *resStr, int i, int j)
{
    appendIndex(resStr, i);
    appendIndex(resStr, j);
}

/*
 * convert varname and integer i to string: "varname[i]"
 * */
void arrayPosToStr(char *resStr, char *varName, int i)
{
    strcpy(resStr, (char *)varName);
    appendIndex(resStr, i);
}

/*
 * convert varname, integer i and integer j to string: "varname[i][j]"
 * */
void array2DPosToStr(char *resStr, char *varName, int i, int j)
{
    strcpy(resStr, varName);
    append2DIndex(resStr, i, j);
}