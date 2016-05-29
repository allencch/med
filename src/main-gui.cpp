/**
 * @author      Allen Choong
 * @version     0.0.1
 * @date        2015-05-25
 *
 * TODO: 2016-05-28
 * Move the mutex to the engine.
 * Add the items to the Med Address
 *
 * TODO
 * Error checking when PID is not selected: Scan button, memValue()
 * "Add All" button
 * Buggy in the sense of multi-threading, and memory accessibility.
 * Suppose to do the exceptional handling separately for the
 * hexToInt() and memValue().
 *
 *
 * DONE
 * Try using multi-threading instead of timeout, because timeout has
 * the problem in the editing-canceled signal from the GtkCellRenderer,
 * when using g_source_remove() in editing-started to stop the timeout,
 * then resume in the editing-canceled will fail. Yet resuming in "edited" signal works fine.
 * So, try to use the multi-threading instead.
 * "Shift" button, Save As, Open,
 */

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>
#include <cinttypes>
#include <dirent.h>
#include <gtk/gtk.h>

#include <json/json.h>


#include "med.hpp"

using namespace std;


Med med; //Global to replace scanner.

static GMutex editMutex;
static string guiStatus;


//Prototype
gpointer refreshAll(gpointer data);


/**
 * Called whenever showing chooseProc window
 */
void refreshProc(GtkListStore* pidStore) {
  gtk_list_store_clear(pidStore);
  //Add in item
  GtkTreeIter iter;
  med.listProcesses();
  for(int i=med.processes.size()-1;i>=0;i--) {
    gtk_list_store_append(pidStore, &iter);
    gtk_list_store_set(pidStore, &iter,
                       0, med.processes[i].pid.c_str(),
                       1, med.processes[i].cmdline.c_str(),
                       -1
                       );
  }
}

/**
 * Called when Process button is clicked
 */
void showProc(GtkButton *button, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;

  GtkWidget *chooseProc = GTK_WIDGET(gtk_builder_get_object(builder,"chooseProc"));

  GtkListStore *  pidStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"pidStore"));
  refreshProc(pidStore);

  gtk_widget_show_all(chooseProc);
}

/**
 * Activated when the chooseProc window is closed
 */
gboolean chooseProcDelete(GtkWidget *window, GdkEvent *event, gpointer data) {
  gtk_widget_hide(window);
  return true;
}

/**
 * Called when a process is double clicked
 */
void procChosen(GtkTreeView *tv, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data) {

  GtkBuilder* builder = (GtkBuilder*) data;
  //Model
  GtkTreeModel *model = gtk_tree_view_get_model(tv);

  //Get the PID from the selected
  GtkTreeSelection *select = gtk_tree_view_get_selection(tv);

  GtkTreeIter iter;
  gtk_tree_selection_get_selected(select,&model,&iter);

  int index = atoi(gtk_tree_model_get_string_from_iter(model, &iter));
  med.selectedProcess = med.processes[med.processes.size() -1 -index];

  //make the changes to the selectedProc, and hide the window
  GtkWidget *selectedProc = GTK_WIDGET(gtk_builder_get_object(builder,"selectedProc"));
  gtk_entry_set_text(GTK_ENTRY(selectedProc), (med.selectedProcess.pid +" "+ med.selectedProcess.cmdline).c_str());

  GtkWidget *procWindow = GTK_WIDGET(gtk_builder_get_object(builder,"chooseProc"));
  gtk_widget_hide(procWindow);
}

void addressToScanStore(Med med, string scanType,GtkListStore *store) {
  GtkTreeIter iter;
  for(int i=0;i<med.scanAddresses.size();i++) {
    char address[32];
    sprintf(address,"%p",(void*)(med.scanAddresses[i].address));

    //Get the value from address and type
    string value = med.getScanAddressValueByIndex(i, scanType);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store,&iter,
                       0, address,
                       1, scanType.c_str(),
                       2, value.c_str(),
                       -1
                       );//*/
  }
}


void updateNumberOfAddresses(GtkBuilder* builder) {
  //Update the status bar
  GtkLabel* status = GTK_LABEL(gtk_builder_get_object(builder, "status"));
  char message[128];
  sprintf(message,"%ld addresses found", med.scanAddresses.size());
  gtk_label_set_text(status, message);
}



/**
 * Called when button is clicked
 * Scan the value based on selected PID.
 */
void scan(GtkButton *button,gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  //Clear the list store
  GtkListStore *scanStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"scanStore"));
  gtk_list_store_clear(scanStore);


  //Get the scanned type
  GtkComboBoxText* combo = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,"scanType"));
  string scanType = gtk_combo_box_text_get_active_text(combo);

  //Get the value
  GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder,"scanEntry"));
  string scanValue = gtk_entry_get_text(entry);

  if(med.selectedProcess.pid == "") { //Default is no string
    cerr << "No process selected" <<endl;
    return;
  }

  try {
    med.scanEqual(scanValue, scanType);
  } catch(string e) {
    cerr<< "scan: "<< e<<endl;
  }


  //Show the results at the scanTreeView
  GtkListStore* store = GTK_LIST_STORE(gtk_builder_get_object(builder, "scanStore"));
  if(med.scanAddresses.size() <= 800) {
    addressToScanStore(med,scanType,store);
  }
  updateNumberOfAddresses(builder);
}



/**
 * Called when filter button is clicked
 * Scan the value based on selected PID.
 */
void filter(GtkButton *button,gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;


  //Clear the list store
  GtkListStore *scanStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"scanStore"));
  gtk_list_store_clear(scanStore);

  //Get the scanned type
  GtkComboBoxText* combo = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder,"scanType"));
  string scanType = gtk_combo_box_text_get_active_text(combo);

  //Get the value
  GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder,"scanEntry"));
  string scanValue = gtk_entry_get_text(entry);

  //Process with scanning
  med.scanFilter(scanValue, scanType);

  //Show the results at the scanTreeView
  GtkListStore* store = GTK_LIST_STORE(gtk_builder_get_object(builder, "scanStore"));
  if(med.scanAddresses.size() <= 800) {
    addressToScanStore(med,scanType,store);
  }

  updateNumberOfAddresses(builder);
}



void createProcessTreeView(GtkBuilder* builder) {
  //Create the list store
  GtkListStore *  pidStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"pidStore"));

  //Get tree view
  GtkWidget* treeview = GTK_WIDGET(gtk_builder_get_object(builder,"procTreeView"));


  //Still need to create columns
  //Create 1st column
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("PID",renderer,"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  //Create second column
  renderer = gtk_cell_renderer_text_new();
  col = gtk_tree_view_column_new_with_attributes("Process",renderer,"text",1,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview),1); //Enable type to search at second column
}



/**
 * This is called when the scan, scanTreeView, scan type is edited. This function suppose is locked
 * @param pathStr is the vertical index based on the selected tree, not the selected item from the combobox
 */
void editScanType(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "scanStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  gtk_list_store_set(model, &iter, 1, text, -1);

  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    g_mutex_unlock(&editMutex);
    return;
  }


  try {
    med.scanAddresses[atoi(pathStr)].setScanType(text);
    string value2 = med.getValueByAddress(med.scanAddresses[atoi(pathStr)].address, text);
    gtk_list_store_set(model, &iter, 2, value2.c_str(), -1);
  } catch(string e) {
    cerr<< "editScanType(): " << e<<endl;
  }

  g_mutex_unlock(&editMutex);
}

void editAddrType(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "addressStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  gtk_list_store_set(model, &iter, 2, text, -1);

  //Get PID
  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    g_mutex_unlock(&editMutex);
    return;
  }


  try {
    med.addresses[atoi(pathStr)].setScanType(text);
    string value2 = med.getValueByAddress(med.addresses[atoi(pathStr)].address, text);
    gtk_list_store_set(model, &iter, 3, value2.c_str(), -1);
  } catch(string e) {
    cout<<"editAddrType: "<< e <<endl;
  }

  g_mutex_unlock(&editMutex);
}


/**
 * This is called when the scan, scanTreeView, value is edited.
 */
void editScanValue(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "scanStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  //Get PID
  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    g_mutex_unlock(&editMutex);
    return;
  }

  try {
    med.setValueByAddress(med.scanAddresses[atoi(pathStr)].address, text, med.scanAddresses[atoi(pathStr)].getScanType());

    //It is suppose to be locked during start editing
    gtk_list_store_set(model, &iter, 2, text, -1);
  } catch(string e) {
    cerr << "editScanValue: "<<e <<endl;
  }
  g_mutex_unlock(&editMutex);
}

void editAddrValue(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "addressStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    //g_mutex_unlock(&memMutex);
    g_mutex_unlock(&editMutex);
    return;
  }

  try {
    med.setValueByAddress(med.addresses[atoi(pathStr)].address, text, med.addresses[atoi(pathStr)].getScanType());

    //It is suppose to be locked during start editing
    gtk_list_store_set(model, &iter, 3, text, -1);
  } catch(string e) {
    cerr<<"editAddrValue: "<<e <<endl;
  }

  g_mutex_unlock(&editMutex);

}

void editScanAddr(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "scanStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);

  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  //First, check the validity of the string
  long address;
  try {
    address = hexToInt(text);
    med.scanAddresses[atoi(pathStr)].address = address;
    gtk_list_store_set(model, &iter, 0, text, -1);
  } catch(string e) {
    cerr<< "editScanAddr: "<< e <<endl;
    g_mutex_unlock(&editMutex);
    return;
  }

  //Now check the validity of the memory

  //Get PID
  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    g_mutex_unlock(&editMutex);
    return;
  }

  try {
    string value2 = med.getValueByAddress(med.scanAddresses[atoi(pathStr)].address, med.scanAddresses[atoi(pathStr)].getScanType());
    gtk_list_store_set(model, &iter, 2, value2.c_str(), -1);

  } catch(string e) {
    gtk_list_store_set(model, &iter, 2, "Error memory", -1);
    cerr<< "editScanAddr.getValueByAddress: "<<e<<endl;
  }

  g_mutex_unlock(&editMutex);
}

/**
 * Edit address panel's address
 */
void editAddrAddr(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "addressStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);

  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  //First, check the validity of the string
  long address;
  try {
    address = hexToInt(text);
    med.addresses[atoi(pathStr)].address = address;
    gtk_list_store_set(model, &iter, 1, text, -1);
  } catch(string e) {
    cerr<< "editAddrAddr: "<< e <<endl;
    //g_mutex_unlock(&memMutex);
    g_mutex_unlock(&editMutex);
    return;
  }

  //Get PID
  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    //g_mutex_unlock(&memMutex);
    g_mutex_unlock(&editMutex);
    return;
  }


  try {
    string value2 = med.getValueByAddress(med.addresses[atoi(pathStr)].address, med.addresses[atoi(pathStr)].getScanType());
    gtk_list_store_set(model, &iter, 3, value2.c_str(), -1);

  } catch(string e) {
    gtk_list_store_set(model, &iter, 3, "Error memory", -1);
    cerr<< "editAddrAddr: "<< e <<endl;
  }

  //g_mutex_unlock(&memMutex);
  g_mutex_unlock(&editMutex);
}

void editAddrDesc(GtkCellRendererText *cell,const gchar *pathStr, const gchar *text, gpointer data) {
  GtkBuilder *builder = (GtkBuilder*) data;
  GtkListStore *model = (GtkListStore*) gtk_builder_get_object(builder, "addressStore");
  GtkTreePath* path = gtk_tree_path_new_from_string(pathStr);
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(model),&iter,path);

  gtk_tree_path_free(path);

  med.addresses[atoi(pathStr)].description = text;

  gtk_list_store_set(model, &iter, 0, text, -1);
  //g_mutex_unlock(&memMutex);
  g_mutex_unlock(&editMutex);
}

void editStart(GtkCellRenderer* renderer, GtkCellEditable* edit, gchar* path, gpointer data) {
  g_mutex_lock(&editMutex);
}

void editCancel(GtkCellRenderer* renderer, GtkCellEditable* edit, gchar* path, gpointer data) {
  g_mutex_unlock(&editMutex);
}

void createScanTreeView(GtkBuilder* builder) {
  //Create the list store
  GtkListStore *  scanStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"scanStore"));

  //Get tree view
  GtkWidget* treeview = GTK_WIDGET(gtk_builder_get_object(builder,"scanTreeView"));

  GtkListStore* scantypeStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"scantypeStore"));


  //Still need to create columns
  //Create 1st column
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer,"editable",true,NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editScanAddr), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Address",renderer,"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  //Create second column
  renderer = gtk_cell_renderer_combo_new();
  g_object_set(renderer,
               "editable",true,
               "text-column", 0, //To show the model
               "model", scantypeStore, //To use the model
               "has-entry", false, //So that no editing
               NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editScanType), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  col = gtk_tree_view_column_new_with_attributes("Type",renderer,"text",1,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);


  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer,"editable",true,NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editScanValue), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  col = gtk_tree_view_column_new_with_attributes("Value",renderer,"text",2,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

}

void lockItem(GtkCellRendererToggle *cell,
              gchar *pathStr,
              gpointer data) {
}

void createAddressTreeView(GtkBuilder* builder) {
  //Create the list store
  GtkListStore *  scanStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"addressStore"));

  //Get tree view
  GtkWidget* treeview = GTK_WIDGET(gtk_builder_get_object(builder,"addressTreeView"));


  //Still need to create columns
  //Create 1st column
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer,"editable",true,NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editAddrDesc), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Description",renderer,"text",0,NULL);
  g_object_set(col,"resizable", true, NULL);

  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer,"editable",true,NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editAddrAddr), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  col = gtk_tree_view_column_new_with_attributes("Address",renderer,"text",1,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  //Create 3rd column
  GtkListStore* scantypeStore = GTK_LIST_STORE(gtk_builder_get_object(builder,"scantypeStore"));

  renderer = gtk_cell_renderer_combo_new();
  g_object_set(renderer,
               "editable",true,
               "text-column", 0, //To show the model
               "model", scantypeStore, //To use the model
               "has-entry", false, //So that no editing
               NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editAddrType), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  col = gtk_tree_view_column_new_with_attributes("Type",renderer,"text",2,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);


  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer,"editable",true,NULL);
  g_signal_connect(renderer,"edited", G_CALLBACK(editAddrValue), builder);
  g_signal_connect(renderer,"editing-started", G_CALLBACK(editStart), builder);
  g_signal_connect(renderer,"editing-canceled", G_CALLBACK(editCancel), builder);
  col = gtk_tree_view_column_new_with_attributes("Value",renderer,"text",3,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),col);

  //Create 5th column: Lock
  renderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(renderer, "toggled", G_CALLBACK(lockItem), builder);
  col = gtk_tree_view_column_new_with_attributes("Lock", renderer, "active", 4, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

}

void refreshScanTreeView(GtkBuilder* builder, pid_t pid) {
  GtkListStore *model = GTK_LIST_STORE(gtk_builder_get_object(builder,"scanStore"));

  GtkTreeIter iter;

  GtkTreePath* path = gtk_tree_path_new_from_string("0");
  gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
  gtk_tree_path_free(path);

  while(gtk_list_store_iter_is_valid(model,&iter)) {

    try {
      int index = atoi(gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(model), &iter));
      string value2 = med.getScanValueByIndex(index);

      //Something wrong with this one.
      gtk_list_store_set(model, &iter, 2, value2.c_str(), -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter);

    } catch(string e) {
      gtk_list_store_set(model, &iter, 2, "Error memory", -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter);
    }
  }
}

void refreshAddressTreeView(GtkBuilder* builder, pid_t pid) throw(string) {
  //GtkTreeView *tv = GTK_TREE_VIEW(gtk_builder_get_object(builder,"addressTreeView"));

  GtkListStore *model = GTK_LIST_STORE(gtk_builder_get_object(builder,"addressStore"));


  GtkTreeIter iter;

  GtkTreePath* path = gtk_tree_path_new_from_string("0");
  gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
  gtk_tree_path_free(path);

  GValue value = G_VALUE_INIT;
  while(gtk_list_store_iter_is_valid(model,&iter)) {
    try {
      int index = atoi(gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(model), &iter));
      string value2 = med.getAddressValueByIndex(index);

      gtk_list_store_set(model, &iter, 3, value2.c_str(), -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter);
    } catch(string e) {
      gtk_list_store_set(model, &iter, 3, "Error memory", -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter);
    }

  }
}



gpointer refreshAll(gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  while(1) {
    g_usleep(800*1000);
    g_mutex_lock(&editMutex);

    try {
      pid_t pid;
      if(med.selectedProcess.pid != "")
        pid = stol(med.selectedProcess.pid);


      //Refresh scan
      refreshScanTreeView(builder, pid);

      //Refresh address
      refreshAddressTreeView(builder, pid);

    } catch(string e) {
      cerr<<"refreshAll(): " << e << endl;
    }
    g_mutex_unlock(&editMutex);
  }

  return NULL;
}

/**
 * @deprecated
 */
gboolean refreshCb(gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;
  try {
    pid_t pid;
    if(med.selectedProcess.pid != "")
      pid = stol(med.selectedProcess.pid);

    //Refresh scan
    refreshScanTreeView(builder, pid);

    //Refresh address
    refreshAddressTreeView(builder, pid);

  } catch(string e) {
    cerr<<"refreshCb(): " << e << endl;
  }
  return true;
}

/**
 * This callback function is called when the scan add button is clicked
 */
void addScanToAddress(GtkWidget* button, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  //Get the selected treeview item
  GtkTreeView *tv = GTK_TREE_VIEW(gtk_builder_get_object(builder, "scanTreeView"));

  GtkTreeModel *model = GTK_TREE_MODEL(gtk_builder_get_object(builder, "scanStore"));

  GtkTreeSelection *select = gtk_tree_view_get_selection(tv);
  GtkTreeIter iter;

  if(! gtk_tree_selection_get_selected(select, &model, &iter) ) {
    return;
  }

  int index = atoi(gtk_tree_model_get_string_from_iter(model, &iter));
  MedAddress medAddress;
  medAddress.address = med.scanAddresses[index].address;
  medAddress.scanType = med.scanAddresses[index].scanType;
  med.addresses.push_back(medAddress);


  //Add to the address panel
  GtkTreeModel *addrModel = GTK_TREE_MODEL(gtk_builder_get_object(builder, "addressStore"));
  GtkTreeIter addrIter;
  gtk_list_store_append(GTK_LIST_STORE(addrModel), &addrIter);
  gtk_list_store_set(GTK_LIST_STORE(addrModel), &addrIter,
                     0, "Your description",
                     1, intToHex(medAddress.address).c_str(),
                     2, medAddress.getScanType().c_str(),
                     3, med.getAddressValueByIndex(med.addresses.size() -1).c_str(),
                     4, false,
                     -1
                     );

}


/**
 * Clear scan
 */
void clearScan(GtkWidget* button, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;


  //Get the selected treeview item
  GtkTreeModel *model = GTK_TREE_MODEL(gtk_builder_get_object(builder, "scanStore"));

  gtk_list_store_clear(GTK_LIST_STORE(model));

  med.scanAddresses.clear();

  GtkLabel* status = GTK_LABEL(gtk_builder_get_object(builder, "status"));
  gtk_label_set_text(status, "Scan cleared");

}

void newAddr(GtkWidget* button, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  GtkTreeModel *model = GTK_TREE_MODEL(gtk_builder_get_object(builder, "addressStore"));


  MedAddress address;
  address.description = "Your description";
  address.address = 0;
  address.setScanType("int32");
  address.lock = false;
  med.addresses.push_back(address);

  GtkTreeIter iter;
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                     0, "Your description",
                     1, "0",
                     2, "int32",
                     4, false,
                     -1
                     );

}

void delAddr(GtkWidget* button, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  //Get the selected treeview item
  GtkTreeView *tv = GTK_TREE_VIEW(gtk_builder_get_object(builder, "addressTreeView"));

  GtkTreeModel *model = GTK_TREE_MODEL(gtk_builder_get_object(builder, "addressStore"));

  GtkTreeSelection *select = gtk_tree_view_get_selection(tv);

  GtkTreeIter iter;
  if(! gtk_tree_selection_get_selected(select, &model, &iter) ) {
    return;
  }

  int index = atoi(gtk_tree_model_get_string_from_iter(model, &iter));
  med.addresses.erase(med.addresses.begin() + index);

  gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
}

bool saveAsFile(GtkBuilder* builder,char* filename) {
  //Read the content from the list store
  GtkListStore *model = GTK_LIST_STORE(gtk_builder_get_object(builder,"addressStore"));

  GtkTreeIter iter;

  GtkTreePath* path = gtk_tree_path_new_from_string("0");
  gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
  gtk_tree_path_free(path);

  med.saveFile(filename);
  return true;
}

void saveAsDialog(GtkMenuItem* item, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;
  GtkWindow* mainWindow = GTK_WINDOW(gtk_builder_get_object(builder,"mainWindow"));

  //g_mutex_lock(&memMutex);
  //while(guiStatus == "bg wrting")
  //  g_cond_wait(&cond,&memMutex);
  GtkWidget* dialog = GTK_WIDGET(gtk_file_chooser_dialog_new("Save As ...",
                                                             mainWindow, GTK_FILE_CHOOSER_ACTION_SAVE,
                                                             "_Cancel", GTK_RESPONSE_CANCEL,
                                                             "_Save", GTK_RESPONSE_ACCEPT, NULL));

  GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
  gtk_file_chooser_set_do_overwrite_confirmation(chooser, true);
  gint res = gtk_dialog_run(GTK_DIALOG(dialog));
  if(res == GTK_RESPONSE_ACCEPT) {
    char* filename;
    filename = gtk_file_chooser_get_filename(chooser);

    saveAsFile(builder,filename);

    g_free(filename);
  }
  gtk_widget_destroy(dialog);

  //g_mutex_unlock(&memMutex);
}

bool openFile(GtkBuilder* builder,char* filename) {
  //Open file
  med.openFile(filename);

  //Read the content from the list store
  GtkListStore *model = GTK_LIST_STORE(gtk_builder_get_object(builder,"addressStore"));

  gtk_list_store_clear(model);


  GtkTreeIter iter;
  for(int i=0;i<med.addresses.size();i++) {
    gtk_list_store_append(model,&iter);
    gtk_list_store_set(model, &iter,
                       0, med.addresses[i].description.c_str(),
                       1, intToHex(med.addresses[i].address).c_str(),
                       2, med.addresses[i].getScanType().c_str(),
                       -1);
  }

  return true;
}

void openFileDialog(GtkMenuItem* item, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;
  GtkWindow* mainWindow = GTK_WINDOW(gtk_builder_get_object(builder,"mainWindow"));

  //g_mutex_lock(&memMutex);
  //while(guiStatus == "bg wrting")
  //  g_cond_wait(&cond,&memMutex);
  GtkWidget* dialog = GTK_WIDGET(gtk_file_chooser_dialog_new("Open ...",
                                                             mainWindow, GTK_FILE_CHOOSER_ACTION_SAVE,
                                                             "_Cancel", GTK_RESPONSE_CANCEL,
                                                             "_Open", GTK_RESPONSE_ACCEPT, NULL));

  GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
  gint res = gtk_dialog_run(GTK_DIALOG(dialog));
  if(res == GTK_RESPONSE_ACCEPT) {
    char* filename;
    filename = gtk_file_chooser_get_filename(chooser);

    openFile(builder,filename);

    g_free(filename);
  }
  gtk_widget_destroy(dialog);

  //g_mutex_unlock(&memMutex);
}


void shiftAddr(GtkButton* button, gpointer data) {
  GtkBuilder* builder = (GtkBuilder*) data;

  //Get the shift from and shift to, then calculate the difference
  GtkEntry* fromEntry = GTK_ENTRY(gtk_builder_get_object(builder,"shiftFrom"));
  GtkEntry* toEntry = GTK_ENTRY(gtk_builder_get_object(builder,"shiftTo"));

  unsigned long shiftFrom, shiftTo, difference;
  try {
    shiftFrom = hexToInt(string(gtk_entry_get_text(fromEntry)));
    shiftTo = hexToInt(string(gtk_entry_get_text(toEntry)));
    difference = shiftTo - shiftFrom;
  } catch(string e) {
    throw e;
  }

  //Shift based on the difference
  GtkListStore *model = GTK_LIST_STORE(gtk_builder_get_object(builder,"addressStore"));

  //Get PID
  if(med.selectedProcess.pid == "") {
    cerr<< "No PID" <<endl;
    return;
  }

  GtkTreeIter iter;

  GtkTreePath* path = gtk_tree_path_new_from_string("0");
  gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
  gtk_tree_path_free(path);

  while(gtk_list_store_iter_is_valid(model,&iter)) {
    int index = atoi(gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(model), &iter));

    long address = med.addresses[index].address;
    address += difference;
    med.addresses[index].address = address;

    gtk_list_store_set(model,&iter,
                       1, intToHex(address).c_str(),
                       -1
                       );



    try {
      string value2 = med.getValueByAddress(address, med.addresses[index].getScanType());

      gtk_list_store_set(model, &iter, 3, value2.c_str(), -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter);
    } catch(string e) {
      gtk_list_store_set(model, &iter, 3, "Error memory", -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(model),&iter); //If error, continue
    }
  }
}


static void activate(GtkApplication *app, gpointer user_data) {
  //Main window
  GtkBuilder* builder = gtk_builder_new_from_file("ui.glade");
  GtkWidget* mainWindow = GTK_WIDGET(gtk_builder_get_object(builder,"mainWindow"));
  gtk_application_add_window(app,GTK_WINDOW(mainWindow));

  //Add signal to the process button
  GtkWidget* buttonProcess = GTK_WIDGET(gtk_builder_get_object(builder,"process"));
  g_signal_connect(buttonProcess,"clicked",G_CALLBACK(showProc),builder);

  createProcessTreeView(builder);
  createScanTreeView(builder);
  createAddressTreeView(builder);


  //Add in delete event to the chooseProc
  GtkWidget *chooseProc = GTK_WIDGET(gtk_builder_get_object(builder,"chooseProc"));
  g_signal_connect(chooseProc, "delete-event", G_CALLBACK(chooseProcDelete),NULL);

  //Add double click event to the treeview
  GtkWidget* treeview = GTK_WIDGET(gtk_builder_get_object(builder,"procTreeView"));
  g_signal_connect(treeview,"row-activated",G_CALLBACK(procChosen),builder);

  //Scan button signal
  GtkWidget* scanButton = GTK_WIDGET(gtk_builder_get_object(builder,"scanButton"));
  g_signal_connect(GTK_BUTTON(scanButton),"clicked",G_CALLBACK(scan),builder);

  //Filter button signal
  GtkWidget* filterButton = GTK_WIDGET(gtk_builder_get_object(builder,"filterButton"));
  g_signal_connect(GTK_BUTTON(filterButton),"clicked",G_CALLBACK(filter),builder);

  //Scan add button
  GtkWidget* addButton = GTK_WIDGET(gtk_builder_get_object(builder, "addButton"));
  g_signal_connect(GTK_BUTTON(addButton), "clicked", G_CALLBACK(addScanToAddress), builder);

  GtkWidget* clearButton = GTK_WIDGET(gtk_builder_get_object(builder, "clearButton"));
  g_signal_connect(GTK_BUTTON(clearButton), "clicked", G_CALLBACK(clearScan), builder);

  GtkWidget* newButton = GTK_WIDGET(gtk_builder_get_object(builder, "newButton"));
  g_signal_connect(GTK_BUTTON(newButton), "clicked", G_CALLBACK(newAddr), builder);

  GtkWidget* deleteButton = GTK_WIDGET(gtk_builder_get_object(builder, "deleteButton"));
  g_signal_connect(GTK_BUTTON(deleteButton), "clicked", G_CALLBACK(delAddr), builder);


  //Quit
  GtkMenuItem* quitMenu = GTK_MENU_ITEM(gtk_builder_get_object(builder, "quitMenu"));
  g_signal_connect_swapped(quitMenu, "activate", G_CALLBACK(gtk_widget_destroy), mainWindow);

  GtkMenuItem* saveAsMenu = GTK_MENU_ITEM(gtk_builder_get_object(builder, "saveAsMenu"));
  g_signal_connect(saveAsMenu,"activate",G_CALLBACK(saveAsDialog),builder);

  GtkMenuItem* openMenu = GTK_MENU_ITEM(gtk_builder_get_object(builder, "openMenu"));
  g_signal_connect(openMenu,"activate",G_CALLBACK(openFileDialog),builder);

  //Shift
  GtkButton* shiftButton = GTK_BUTTON(gtk_builder_get_object(builder, "shiftButton"));
  g_signal_connect(shiftButton,"clicked",G_CALLBACK(shiftAddr),builder);


  g_thread_new("refreshAll", refreshAll, builder);
  //g_timeout_add(800, refreshCb, builder);

  gtk_widget_show_all(mainWindow);

}

int main(int argc, char** argv) {
  GtkApplication* app;
  int status;

  app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
