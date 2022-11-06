#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

struct Item {
  int idx;
  char name[20];
  float price;
  int count;
  GtkWidget *count_lable;
};

struct Order {
  int item_idx[20];
  int quantity[20];
  int item_count;
  int order_id;
};

int order_id_g = 100;
static GArray *cart_array = NULL;
static GArray *order_array = NULL;

void update_total_lable();

GtkApplication *app;
GtkWidget *cart_list_box;
GtkWidget *total_lable;
GtkWidget *confirm_button;
GtkWidget *place_order_window;
GtkWidget *token_window;
GtkWidget *big_screen_list_box;
GtkWidget *kitchen_list_box;

struct Item items[100];
int items_len = 0;

void read_items() {
  FILE *ptr = fopen("db.txt", "r");
  if (ptr == NULL) {
    printf("no such file.");
    exit(1);
  }
  char name[20];
  float price;
  while (fscanf(ptr, "%s%f", name, &price)) {
    if (!strcmp(name, "end"))
      break;
    strcpy(items[items_len].name, name);
    items[items_len].price = price;
    items[items_len].idx = items_len;
    items[items_len].count = 0;
    ++items_len;
  }
}

int get_idx_form_order_id(int order_id) {
  for (int i = 0; i < order_array->len; ++i) {
    struct Order *o = order_array->data;
    if (order_id == o[i].order_id) {
      return i;
    }
  }
  return -1;
}
void notify_button_click(GtkButton *button, gpointer order_id) {}
void finish_button_click(GtkButton *button, gpointer order_id) {
  int idx = get_idx_form_order_id((int)order_id);

  GtkListBoxRow *r = gtk_list_box_get_row_at_index(kitchen_list_box, idx);
  gtk_list_box_remove(kitchen_list_box, r);
  g_array_remove_index(order_array, idx);
}
// add order to the screen and kitchen
void add_order(struct Order order) {
  GtkWidget *vbox1;
  GtkWidget *finish_button;
  GtkWidget *notify_button;
  char text[100];

  g_array_append_vals(order_array, &order, 1);
  sprintf(text, "Order id: %d", order.order_id);
  vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  gtk_box_append(vbox1, gtk_label_new(text));

  for (int i = 0; i < order.item_count; ++i) {
    sprintf(text, "%s %d pcs.", items[order.item_idx[i]].name,
            items[order.item_idx[i]].count);
    gtk_box_append(vbox1, gtk_label_new(text));
  }
  notify_button = gtk_button_new_with_label("Notify");
  finish_button = gtk_button_new_with_label("Finish order");

  g_signal_connect(notify_button, "clicked", G_CALLBACK(notify_button_click),
                   order.order_id);
  g_signal_connect(finish_button, "clicked", G_CALLBACK(finish_button_click),
                   order.order_id);
  gtk_box_append(vbox1, notify_button);
  gtk_box_append(vbox1, finish_button);
  gtk_list_box_insert(GTK_LIST_BOX(kitchen_list_box), vbox1, -1);
}

int get_item_idx_from_cart(int idx) {
  int *n = (int *)cart_array->data;
  for (int i = 0; i < cart_array->len; ++i) {
    if (n[i] == idx) {
      return i;
    }
  }
  return -1;
}

void print_button_click(GtkButton *button, gpointer window) {
  int *n = (int *)cart_array->data;

  //======================================

  struct Order order;
  order.order_id = order_id_g;
  order.item_count = cart_array->len;

  //======================================
  int i = 0;
  while (cart_array->len) {
    order.item_idx[i] = n[0];
    order.quantity[i] = items[n[0]].count;
    g_print("len: %d\n", cart_array->len);
    GtkListBoxRow *r = gtk_list_box_get_row_at_index(cart_list_box, 0);
    gtk_list_box_remove(cart_list_box, r);
    g_array_remove_index(cart_array, 0);
    ++i;
  }
  add_order(order);
  items[n[0]].count = 0;
  update_total_lable();
  gtk_window_close(token_window);
}

void add_button_click(GtkButton *button, gpointer user_data) {
  struct Item *item = user_data;
  ++item->count;
  char cout_text[30];
  sprintf(cout_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
          item->price * item->count);
  gtk_label_set_text(item->count_lable, cout_text);
  update_total_lable();
  g_print("add: %s\n", item->name);
}
void show_token_window(GtkButton *button, gpointer user_data) {
  if (!cart_array->len) {
    return;
  }
  GtkWidget *print_button;
  GtkWidget *view;
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

  view = gtk_text_view_new();
  gtk_text_view_set_editable(view,FALSE);
  gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 20);
  gtk_widget_set_vexpand(view, TRUE);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  char line[150];
  int *n = (int *)cart_array->data;
  float total = 0;
  ++order_id_g;
  sprintf(line, "\t\t\t\tOrder ID: %d\n\n", order_id_g);
  gtk_text_buffer_insert(buffer, &iter, line, -1);
  for (int i = 0; i < cart_array->len; ++i) {
    int idx = n[i];
    total += items[idx].price * items[idx].count;
    sprintf(line, "%s\t\t\t\t%.2f x %d\t\t=\t\t%.2f TK\n", items[idx].name,
            items[idx].price, items[idx].count,
            items[idx].price * items[idx].count);
    gtk_text_buffer_insert(buffer, &iter, line, -1);
  }

  gtk_text_buffer_insert(buffer, &iter,
                         "-----------------------------------------------------"
                         "-----------------------------\n",
                         -1);
  sprintf(line, "\t\t\t\t\t\t\t\t\tTotal: %.2f TK\n", total);
  gtk_text_buffer_insert(buffer, &iter, line, -1);

  print_button = gtk_button_new_with_label("Print Token");

  g_signal_connect(print_button, "clicked", G_CALLBACK(print_button_click),
                   NULL);
  token_window = gtk_application_window_new(app);
  gtk_window_set_modal(token_window, TRUE);
  gtk_window_set_transient_for(token_window, place_order_window);
  gtk_window_set_title(GTK_WINDOW(token_window), "Confirm order");
  gtk_window_set_display(GTK_WINDOW(token_window),
                         gtk_widget_get_display(place_order_window));
  gtk_box_append(vbox, view);
  gtk_box_append(vbox, print_button);
  gtk_window_set_child(GTK_WINDOW(token_window), vbox);
  gtk_window_set_default_size(token_window, 500, 700);
  gtk_widget_show(token_window);
}

void remove_button_click(GtkButton *button, gpointer user_data) {
  struct Item *item = user_data;
  --item->count;
  if (item->count > 0) {
    char cout_text[30];
    sprintf(cout_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
            item->price * item->count);
    gtk_label_set_text(item->count_lable, cout_text);
  } else {
    item->count = 0;
    g_print("=========> %d to remove\n", item->idx);
    int cart_idx = get_item_idx_from_cart(item->idx);
    GtkListBoxRow *r = gtk_list_box_get_row_at_index(cart_list_box, cart_idx);
    gtk_list_box_remove(cart_list_box, r);
    g_array_remove_index(cart_array, cart_idx);
  }
  update_total_lable();
  g_print("remove: %s\n", item->name);
}

void add_to_cart(struct Item *item) {
  if (get_item_idx_from_cart(item->idx) >= 0) {
    // increase the count

  } else {
    item->count++;
    GtkWidget *lable;
    GtkWidget *grid;
    GtkWidget *buttonadd;
    GtkWidget *buttonsub;
    GtkWidget *image;
    char image_name[30];
    sprintf(image_name, "images/%s.png", item->name);
    image = gtk_image_new_from_file(image_name);
    gtk_image_set_pixel_size(image, 150);
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(grid, 10);
    gtk_grid_set_column_spacing(grid, 10);

    char cout_text[30];
    sprintf(cout_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
            item->price * item->count);
    item->count_lable = gtk_label_new(cout_text);

    gtk_grid_attach(grid, image, 0, 0, 2, 4);
    gtk_grid_attach(grid, gtk_label_new(item->name), 2, 0, 1, 1);

    gtk_grid_attach(grid, item->count_lable, 2, 1, 3, 1);

    buttonsub = gtk_button_new_with_label("-");
    gtk_widget_set_halign(buttonsub, GTK_ALIGN_END);
    g_signal_connect(buttonsub, "clicked", G_CALLBACK(remove_button_click),
                     item);
    gtk_grid_attach(grid, buttonsub, 2, 2, 1, 1);

    buttonadd = gtk_button_new_with_label("+");

    g_signal_connect(buttonadd, "clicked", G_CALLBACK(add_button_click), item);

    gtk_grid_attach(grid, buttonadd, 3, 2, 1, 1);
    gtk_list_box_insert(GTK_LIST_BOX(cart_list_box), grid, -1);
    g_array_append_vals(cart_array, (int *)&item->idx, 1);
    update_total_lable();
  }
}

void add_to_cart_button_click(GtkButton *button, gpointer user_data) {
  add_to_cart(user_data);
}

void add_items(GtkWidget *listbox) {
  GtkWidget *lable;
  GtkWidget *grid;
  GtkWidget *image;
  GtkWidget *button;

  for (int i = 0; i < items_len; ++i) {
    button = gtk_button_new_with_label("Add to cart");
    g_signal_connect(button, "clicked", G_CALLBACK(add_to_cart_button_click),
                     &items[i]);
    char image_name[30];
    sprintf(image_name, "images/%s.png", items[i].name);
    image = gtk_image_new_from_file(image_name);
    gtk_image_set_pixel_size(image, 150);
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(grid, 10);
    gtk_grid_set_column_spacing(grid, 10);

    gtk_grid_attach(grid, image, 0, 0, 2, 4);
    char p[20];
    sprintf(p, "Price: %.2f TK", items[i].price);
    lable = gtk_label_new(items[i].name);
    gtk_grid_attach(grid, lable, 2, 0, 1, 1);
    lable = gtk_label_new(p);
    gtk_grid_attach(grid, lable, 2, 1, 1, 1);
    gtk_grid_attach(grid, button, 2, 2, 1, 1);

    gtk_list_box_insert(GTK_LIST_BOX(listbox), grid, -1);
  }
}

void item_select_handler(GtkListBox *box, GtkListBoxRow *row,
                         gpointer user_data) {
  int idx = gtk_list_box_row_get_index(row);
  g_print("item_selected: %s\n", items[idx].name);
  // add_to_cart(&items[idx]);
}

void update_total_lable() {
  int *n = (int *)cart_array->data;
  float total = 0;
  for (int i = 0; i < cart_array->len; ++i) {
    int element = n[i];
    total += items[element].price * items[element].count;
  }
  char total_text[20];
  sprintf(total_text, "Total: %.2f", total);
  gtk_label_set_text(total_lable, total_text);
}

void init_big_screen_window(GtkWidget *window) {
  GtkWidget *listbox = gtk_list_box_new();
  big_screen_list_box = listbox;
  GtkWidget *scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_has_frame(scrolled, TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scrolled, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
  gtk_window_set_child(GTK_WINDOW(window), scrolled);
}
void init_kitchen_window(GtkWidget *window) {
  GtkWidget *listbox = gtk_list_box_new();
  kitchen_list_box = listbox;
  GtkWidget *scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_has_frame(scrolled, TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scrolled, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
  gtk_window_set_child(GTK_WINDOW(window), scrolled);
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *hbox;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *grid = gtk_grid_new();
  GtkWidget *grid2 = gtk_grid_new();
  GtkWidget *button;
  GtkWidget *buttonx;
  GtkWidget *button2;
  GtkWidget *scrolled = gtk_scrolled_window_new();
  GtkWidget *scrolled2 = gtk_scrolled_window_new();
  GtkWidget *listbox = gtk_list_box_new();
  GtkWidget *listbox2 = gtk_list_box_new();
  gtk_scrolled_window_set_has_frame(scrolled, TRUE);
  gtk_scrolled_window_set_has_frame(scrolled2, TRUE);

  cart_list_box = listbox2;
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_vexpand(scrolled, TRUE);
  gtk_widget_set_vexpand(scrolled2, TRUE);

  add_items(listbox);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled2), listbox2);

  //=======================
  GtkWidget *big_screen_window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(big_screen_window), "Order queue");
  init_big_screen_window(big_screen_window);

  GtkWidget *kitchen_window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(kitchen_window), "Kitchen Window");
  init_kitchen_window(kitchen_window);
  //=========

  place_order_window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(place_order_window), "Order Window");

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  gtk_box_set_homogeneous(hbox, TRUE);
  vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  // gtk_box_set_homogeneous(vbox1, TRUE);
  vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  // gtk_box_set_homogeneous(vbox2, TRUE);

  // button2 = gtk_button_new_with_label("Button 2");
  // g_signal_connect(button2, "clicked", G_CALLBACK(print_hello), NULL);

  gtk_box_append(GTK_BOX(vbox1), gtk_label_new("MENU:"));
  gtk_box_append(GTK_BOX(vbox2), gtk_label_new("CART:"));

  gtk_box_append(GTK_BOX(vbox1), scrolled);
  gtk_box_append(GTK_BOX(vbox2), scrolled2);

  total_lable = gtk_label_new("");
  confirm_button = gtk_button_new_with_label("Confirm order");
  g_signal_connect(confirm_button, "clicked", G_CALLBACK(show_token_window),
                   NULL);

  gtk_box_append(vbox2, total_lable);
  gtk_box_append(vbox2, confirm_button);
  gtk_box_append(GTK_BOX(hbox), vbox1);
  // gtk_box_append(GTK_BOX(hbox), seperator);
  gtk_box_append(GTK_BOX(hbox), vbox2);

  gtk_window_set_child(GTK_WINDOW(place_order_window), hbox);
  gtk_window_set_default_size(place_order_window, 1000, 800);
  gtk_widget_show(place_order_window);
  gtk_window_set_default_size(big_screen_window, 1000, 800);
  //gtk_widget_show(big_screen_window);
  gtk_window_set_default_size(kitchen_window, 1000, 800);
  //gtk_widget_show(kitchen_window);
}

int main(int argc, char **argv) {
  read_items();
  cart_array = g_array_sized_new(FALSE, FALSE, sizeof(int), 1);
  order_array = g_array_sized_new(FALSE, FALSE, sizeof(struct Item), 1);
  int status;

  app = gtk_application_new("com.sdp.food", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
