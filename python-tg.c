#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_PYTHON

#include "include.h"
#include <string.h>
#include <stdlib.h>

#include "python-tg.h"
#include <Python/Python.h>

#include "structures.h"
#include "interface.h"
#include "constants.h"
#include "tools.h"
#include "queries.h"
#include "net.h"

PyObject *pModule;

void python_init (const char *file) {
    PyObject *pName, *pFunc, *pValue;

    Py_SetProgramName("tg");  /* optional but recommended */
    Py_Initialize();
    PySys_SetPath(".");

    pName = PyString_FromString(file);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        logprintf("module %s\n", file);
        pFunc = PyObject_GetAttrString(pModule, "on_init");
        if (pFunc && PyCallable_Check(pFunc)) {
            pValue = PyObject_CallObject(pFunc, NULL);
            if (pValue != NULL) {
                printf("Result of call: %ld\n", PyInt_AsLong(pValue));
                Py_DECREF(pValue);
            }
        } else {
            if (PyErr_Occurred()) PyErr_Print();
        }
        Py_XDECREF(pFunc);


    } else {
        logprintf("no module %s\n",file);

        PyErr_Print();
        logprintf("Call failed\n");

    }

}
/*
void xxpush_message (struct message *M) {
  assert (M);
  my_lua_checkstack (luaState, 10);
  lua_newtable (luaState);

  if (get_peer_type (M->fwd_from_id)) {
    lua_pushstring (luaState, "fwd_from");
    push_peer (M->fwd_from_id, user_chat_get (M->fwd_from_id));
    lua_settable (luaState, -3); // fwd_from

    lua_add_num_field ("fwd_date", M->fwd_date);
  }

  lua_pushstring (luaState, "from");
  push_peer (M->from_id, user_chat_get (M->from_id));
  lua_settable (luaState, -3);

  lua_pushstring (luaState, "to");
  push_peer (M->to_id, user_chat_get (M->to_id));
  lua_settable (luaState, -3);

  if (!M->service) {
    if (M->message_len && M->message) {
      lua_pushstring (luaState, "text");
      lua_pushlstring (luaState, M->message, M->message_len);
      lua_settable (luaState, -3);
    }
    if (M->media.type  && M->media.type != CODE_message_media_empty && M->media.type != CODE_decrypted_message_media_empty) {
      lua_pushstring (luaState, "media");
      push_media (&M->media);
      lua_settable (luaState, -3);
    }
  }
}
*/

PyObject *get_peer(peer_id_t id)
{
    peer_t *P = user_chat_get(id);
    PyObject *peer = PyDict_New(), *str;
    printf("%p\n", P);
    if (P && P->print_name) {
        logprintf("print_name %s\n",P->print_name);
        str = PyString_FromString(P->print_name);
        PyDict_SetItemString(peer, "print_name", str);
        Py_DECREF(str);
    }

    return peer;
}

PyObject *get_message(struct message *M) {
    PyObject *po;
    static char s[30];
    tsnprintf (s, 30, "%lld", M->id);

    po = Py_BuildValue("({sssssisisisisNsN})",
        "id",s,
        "message",M->message,
        "service",M->service,
        "date",M->date,
        "unread",M->unread,
        "out",M->out,
        "to",get_peer(M->to_id),
        "from",get_peer(M->to_id)
        );

    return po;
}

void python_new_msg(struct message *M UU) {
    PyObject *pFunc, *pValue, *pArgs;
    if (!pModule) { return; }

    logprintf("message %p\n",M);
    pArgs = NULL;
    pFunc = PyObject_GetAttrString(pModule, "on_message_receive");
    if (pFunc && PyCallable_Check(pFunc)) {
        pArgs =  get_message(M);
        pValue = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (pValue != NULL) {
            printf("Result of call: %ld\n", PyInt_AsLong(pValue));
            Py_DECREF(pValue);
        } else {
            if (PyErr_Occurred()) PyErr_Print();
            printf("kein result\n");
        }
    } else {
        if (PyErr_Occurred()) PyErr_Print();
    }
    Py_XDECREF(pFunc);
    logprintf("done\n");
}

void python_finalize() {
    Py_XDECREF(pModule);
    Py_Finalize();
}


#endif
