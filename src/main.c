#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

struct Item {
  int idx;
  char name[20];
  float price;
  int count;
  GtkWidget *count_label;
};

struct Order {
  int item_idx[20];
  int quantity[20];
  int item_count;
  int order_id;
  GtkWidget *bar;
};

int order_id_g = 100;
struct Item items[100];
int items_len = 0;
int order_id_update = 0;

static GArray *cart_array = NULL;
static GArray *order_array = NULL;

GtkApplication *app;
GtkWidget *cart_list_box;
GtkWidget *total_label;
GtkWidget *confirm_button;
GtkWidget *place_order_window;
GtkWidget *token_window;
GtkWidget *big_screen_list_box;
GtkWidget *kitchen_list_box;

void update_total_label();

void read_items() {
  FILE *ptr = fopen("db.txt", "r");
  if (ptr == NULL) {
    printf("%s", "Failed to find the product database file.");
    exit(1);
  }
  char name[20];
  float price;
  while (fscanf(ptr, "%s%f", name, &price) != EOF) {
    strcpy(items[items_len].name, name);
    items[items_len].price = price;
    items[items_len].idx = items_len;
    items[items_len].count = 0;
    ++items_len;
  }
}

void show_msg(const char text[]) {
  gtk_widget_show(gtk_message_dialog_new(
      NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, text));
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

void add_button_click(GtkButton *button, gpointer user_data) {
  struct Item *item = user_data;
  ++item->count;

  char count_text[30];
  sprintf(count_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
          item->price * item->count);
  gtk_label_set_text(item->count_label, count_text);
  update_total_label();
  g_print("add: %s\n", item->name);
}

/*
  Cart Layout
*/

void remove_button_click(GtkButton *button, gpointer user_data) {
  struct Item *item = user_data;
  --item->count;
  if (item->count > 0) {
    char count_text[30];
    sprintf(count_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
            item->price * item->count);
    gtk_label_set_text(item->count_label, count_text);
  } else {
    item->count = 0;
    g_print("=========> %d to remove\n", item->idx);
    int cart_idx = get_item_idx_from_cart(item->idx);
    GtkListBoxRow *r = gtk_list_box_get_row_at_index(cart_list_box, cart_idx);
    gtk_list_box_remove(cart_list_box, r);
    g_array_remove_index(cart_array, cart_idx);
  }
  update_total_label();
  g_print("remove: %s\n", item->name);
}

void discard_button_click(GtkButton *button, gpointer user_data) {
  struct Item *item = user_data;

  item->count = 0;
  int cart_idx = get_item_idx_from_cart(item->idx);

  GtkListBoxRow *r = gtk_list_box_get_row_at_index(cart_list_box, cart_idx);
  gtk_list_box_remove(cart_list_box, r);
  g_array_remove_index(cart_array, cart_idx);
  update_total_label();
  g_print("remove: %s\n", item->name);
}

void add_to_cart(struct Item *item) {
  if (get_item_idx_from_cart(item->idx) >= 0) {
    show_msg("Item already exists in the cart..");
    // increase the count

  } else {
    item->count++;
    GtkWidget *label;
    GtkWidget *grid;
    GtkWidget *button_add;
    GtkWidget *button_remove;
    GtkWidget *button_discard;
    GtkWidget *image;

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(grid, 10);
    gtk_grid_set_column_spacing(grid, 10);
    // gtk_widget_set_size_request(grid, 10, 5);

    // cart item image
    char image_name[30];
    sprintf(image_name, "images/%s.png", item->name);
    image = gtk_image_new_from_file(image_name);
    gtk_image_set_pixel_size(image, 150);

    gtk_grid_attach(grid, image, 0, 0, 2, 4);

    // cart item name
    label = gtk_label_new(item->name);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(grid, label, 2, 0, 1, 1);

    // cart item price
    char count_text[30];
    sprintf(count_text, "Price: %.2f × %d = %.2f TK", item->price, item->count,
            item->price * item->count);
    item->count_label = gtk_label_new(count_text);
    gtk_grid_attach(grid, item->count_label, 2, 1, 3, 1);

    // cart button add
    button_add = gtk_button_new_with_label("+");
    gtk_widget_set_halign(button_add, GTK_ALIGN_CENTER);
    g_signal_connect(button_add, "clicked", G_CALLBACK(add_button_click), item);
    gtk_grid_attach(grid, button_add, 2, 2, 1, 1);

    // cart button remove
    button_remove = gtk_button_new_with_label("-");
    gtk_widget_set_halign(button_remove, GTK_ALIGN_CENTER);
    g_signal_connect(button_remove, "clicked", G_CALLBACK(remove_button_click),
                     item);
    gtk_grid_attach(grid, button_remove, 3, 2, 1, 1);

    // cart button discard
    button_discard = gtk_button_new_with_label("x");
    gtk_widget_set_halign(button_discard, GTK_ALIGN_CENTER);
    g_signal_connect(button_discard, "clicked",
                     G_CALLBACK(discard_button_click), item);
    gtk_grid_attach(grid, button_discard, 4, 2, 1, 1);

    gtk_list_box_insert(GTK_LIST_BOX(cart_list_box), grid, -1);
    g_array_append_vals(cart_array, (int *)&item->idx, 1);
    update_total_label();
  }
}

void update_total_label() {
  int *n = (int *)cart_array->data;
  float total = 0;
  for (int i = 0; i < cart_array->len; ++i) {
    int element = n[i];
    total += items[element].price * items[element].count;
  }
  char total_text[20];
  sprintf(total_text, "Total: %.2f Tk", total);
  gtk_label_set_text(total_label, total_text);
}

/*
  Item Dashboard
*/

void add_to_cart_button_click(GtkButton *button, gpointer user_data) {
  add_to_cart(user_data);
}

//add items to the cart form items array
void add_items(GtkWidget *listbox) {
  GtkWidget *label;
  GtkWidget *grid;
  GtkWidget *image;
  GtkWidget *button;

  for (int i = 0; i < items_len; ++i) {
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(grid, 10);
    gtk_grid_set_column_spacing(grid, 10);
    // gtk_widget_set_size_request(grid, 10, 3);

    // item name
    label = gtk_label_new(items[i].name);
    gtk_grid_attach(grid, label, 2, 0, 1, 1);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    // item price
    char item_price[20];
    sprintf(item_price, "Price: %.2f TK", items[i].price);
    label = gtk_label_new(item_price);
    gtk_grid_attach(grid, label, 2, 1, 1, 1);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    // item image
    char image_path[30];
    sprintf(image_path, "images/%s.png", items[i].name);
    image = gtk_image_new_from_file(image_path);
    gtk_image_set_pixel_size(image, 150);
    gtk_grid_attach(grid, image, 0, 0, 2, 4);

    // Add to cart button
    button = gtk_button_new_with_label("Add to cart");
    g_signal_connect(button, "clicked", G_CALLBACK(add_to_cart_button_click),
                     &items[i]);
    gtk_grid_attach(grid, button, 2, 2, 1, 1);

    gtk_list_box_insert(GTK_LIST_BOX(listbox), grid, -1);
  }
}

void item_select_handler(GtkListBox *box, GtkListBoxRow *row,
                         gpointer user_data) {
  int idx = gtk_list_box_row_get_index(row);
  g_print("item_selected: %s\n", items[idx].name);
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

/*
  Kitchen Window
*/

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

int get_idx_form_order_id(int order_id) {
  for (int i = 0; i < order_array->len; ++i) {
    struct Order *o = order_array->data;
    g_print("order_id %d\n", o[i].order_id);
    if (order_id == o[i].order_id) {
      return i;
    }
  }
  return -1;
}
void notify_button_click(GtkButton *button, gpointer order_id) {
  int idx = get_idx_form_order_id((long long)order_id);
  struct Order *o = order_array->data;
  gtk_info_bar_set_message_type(GTK_INFO_BAR(o[idx].bar), GTK_MESSAGE_QUESTION);
}
void finish_button_click(GtkButton *button, gpointer order_id) {
  g_print("pointer to order_id %lld\n", (long long)order_id);
  int idx = get_idx_form_order_id((long long)order_id);

  g_print("%d\n", idx);

  GtkListBoxRow *r = gtk_list_box_get_row_at_index(kitchen_list_box, idx);
  GtkListBoxRow *r2 = gtk_list_box_get_row_at_index(big_screen_list_box, idx);
  gtk_list_box_remove(kitchen_list_box, r);
  gtk_list_box_remove(big_screen_list_box, r2);
  g_array_remove_index(order_array, idx);
}

// add order to the screen and kitchen
void add_order(struct Order order) {
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *vbox_inner;
  GtkWidget *bar;
  GtkWidget *label;
  GtkWidget *finish_button;
  GtkWidget *notify_button;
  char text[100];

  vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  vbox_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

  bar = gtk_info_bar_new();
  order.bar = bar;
  gtk_box_append(GTK_BOX(vbox2), bar);
  gtk_info_bar_set_message_type(GTK_INFO_BAR(bar), GTK_MESSAGE_WARNING);

  g_print("adding order id %d", order.order_id);
  g_array_append_vals(order_array, &order, 1);
  sprintf(text, "Order id: %d", order.order_id);
  gtk_box_append(vbox1, gtk_label_new(text));
  // gtk_grid_attach(vbox1, image, 0, 0, 2, 4);
  gtk_box_append(vbox_inner, gtk_label_new(text));

  for (int i = 0; i < order.item_count; ++i) {
    sprintf(text, "%s %d pcs.", items[order.item_idx[i]].name,
            items[order.item_idx[i]].count);
    gtk_box_append(vbox1, gtk_label_new(text));
    gtk_box_append(vbox_inner, gtk_label_new(text));
    // gtk_info_bar_add_child(GTK_INFO_BAR(bar), gtk_label_new(text));

    items[order.item_idx[i]].count = 0;
  }
  gtk_info_bar_add_child(GTK_INFO_BAR(bar), vbox_inner);
  notify_button = gtk_button_new_with_label("Notify");
  finish_button = gtk_button_new_with_label("Finish order");

  order_id_update = order.order_id;
  g_signal_connect(notify_button, "clicked", G_CALLBACK(notify_button_click),
                   order.order_id);
  g_signal_connect(finish_button, "clicked", G_CALLBACK(finish_button_click),
                   order.order_id);
  gtk_box_append(vbox1, notify_button);
  gtk_box_append(vbox1, finish_button);
  gtk_list_box_insert(GTK_LIST_BOX(kitchen_list_box), vbox1, -1);
  gtk_list_box_insert(GTK_LIST_BOX(big_screen_list_box), vbox2, -1);
}

/*
  Token Window
*/

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
  update_total_label();
  gtk_window_close(token_window);
}

void show_token_window(GtkButton *button, gpointer user_data) {
  token_window = gtk_application_window_new(app);

  GtkWidget *label;
  GtkWidget *grid;
  grid = gtk_grid_new();
  gtk_grid_set_row_spacing(grid, 10);
  gtk_grid_set_column_spacing(grid, 10);

  if (!cart_array->len) {
    show_msg("Cart is empty.");
    return;
  }

  GtkWidget *print_button;
  GtkWidget *view;
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

  view = gtk_text_view_new();
  gtk_text_view_set_editable(view, FALSE);
  gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 20);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
  gtk_widget_set_vexpand(view, TRUE);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  char line[150];
  int *n = (int *)cart_array->data;
  float total = 0;
  ++order_id_g;
  sprintf(line, "\t\t     Order ID: %d\n\n\n", order_id_g);
  gtk_text_buffer_insert(buffer, &iter, line, -1);
  for (int i = 0; i < cart_array->len; ++i) {
    int idx = n[i];

    // adjusting space in token window
    char name_space[100] = "                      ";
    if ((8 - strlen(items[idx].name)) == 1) {
      strcat(name_space, " ");
    } else if ((8 - strlen(items[idx].name)) == 2) {
      strcat(name_space, "  ");
    } else if ((8 - strlen(items[idx].name)) == 3) {
      strcat(name_space, "   ");
    } else if ((8 - strlen(items[idx].name)) == 4) {
      strcat(name_space, "    ");
    } else if ((8 - strlen(items[idx].name)) == 5) {
      strcat(name_space, "     ");
    }

    char price_space[4] = " ";
    if (items[idx].price >= 10.00 && items[idx].price < 100.00) {
      strcat(price_space, " ");
    } else if (items[idx].price < 10.00) {
      strcat(price_space, "  ");
    }

    char total_space[4] = " ";
    if ((items[idx].price * items[idx].count) >= 10.00 &&
        (items[idx].price * items[idx].count) < 100.00) {
      strcat(total_space, " ");
    } else if ((items[idx].price * items[idx].count) < 10.00) {
      strcat(total_space, "  ");
    }
    strcat(name_space, price_space);

    total += items[idx].price * items[idx].count;
    sprintf(line, "  %s%s%0.2f x %d =%s%0.2f TK\n", items[idx].name, name_space,
            items[idx].price, items[idx].count, total_space,
            items[idx].price * items[idx].count);
    gtk_text_buffer_insert(buffer, &iter, line, -1);
  }

  gtk_text_buffer_insert(
      buffer, &iter,
      "--------------------------------------------------------\n", -1);
  sprintf(line, "\t\t\t\t      Total = %06.2f TK\n", total);
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
  gtk_window_set_default_size(token_window, 490, 700);
  gtk_widget_show(token_window);
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

  total_label = gtk_label_new("");
  confirm_button = gtk_button_new_with_label("Confirm order");
  g_signal_connect(confirm_button, "clicked", G_CALLBACK(show_token_window),
                   NULL);
  gtk_box_append(vbox2, total_label);
  gtk_box_append(vbox2, confirm_button);
  gtk_box_append(GTK_BOX(hbox), vbox1);
  // gtk_box_append(GTK_BOX(hbox), seperator);
  gtk_box_append(GTK_BOX(hbox), vbox2);

  gtk_window_set_child(GTK_WINDOW(place_order_window), hbox);
  gtk_window_set_default_size(place_order_window, 1000, 800);
  gtk_widget_show(place_order_window);
  gtk_window_set_default_size(big_screen_window, 350, 800);
  gtk_widget_show(big_screen_window);
  gtk_window_set_default_size(kitchen_window, 350, 800);
  gtk_widget_show(kitchen_window);
}

int main(int argc, char **argv) {
  read_items();
  cart_array = g_array_sized_new(FALSE, FALSE, sizeof(int), 1);
  order_array = g_array_sized_new(FALSE, FALSE, sizeof(struct Order), 1);
  int status;

  app = gtk_application_new("com.sdp.food", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
