#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>

#define EMPTY   0
#define WHITE   -1
#define BLACK   1
#define HEXAGON	7
#define SIZE  	4

int ** board;
int ** boardPlayer1;
int * groupsPlayer1;
int ** boardPlayer2;
int * groupsPlayer2;
int belong;
int player = WHITE;
int num_hum_players = 1;
int hum_player = WHITE;
int turn = 1;
void checkBoard(int i, int j);
void generateBoard(void);
static void draw_board(cairo_t *);
void first_player_strategy(void);
int hexcell[HEXAGON][2] = { {68,30},
	{51,60},
	{17,60},
	{0,30},
	{17,0},
	{51,0},
	{68,30}
};
int points[HEXAGON][2] = { {0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};

GdkPixbuf *create_pixbuf(const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void clicked_call_back(GtkButton *b, GtkRadioButton *rb[][SIZE])
{
	int i,j;
	if(isThereAWinner())	return;
	GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(b));
	if(rb[0][0] != NULL) {
		for(i = 0;i<SIZE && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb[0][i])); ++i)
			;
		for(j = 0;j<SIZE && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb[1][j])); ++j)
			;
	
	
		if(board[j][i] == 0) {
		   	if(num_hum_players == 2) {
				board[j][i] = player;
				checkBoard(j,i);
				player = player == WHITE ? BLACK : WHITE;
				gtk_widget_queue_draw(win);
			} 
			else {
				turn++;
				board[j][i] = BLACK;
				player = player == WHITE ? BLACK : WHITE;
				checkBoard(j,i);
				turn++;
				first_player_strategy();
				player = player == WHITE ? BLACK : WHITE;
				gtk_widget_queue_draw(win);
			}
		}
	} else 
		while(TRUE) {
			//machine_play(WHITE);
			gtk_widget_queue_draw(win);
			//machine_play(BLACK);
			gtk_widget_queue_draw(win);
		}	
	
			
}

void new_game_call_back(GSimpleAction *simple,
          GVariant      *parameter,
          gpointer       user_data)
{
	generateBoard();
	player = WHITE;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr,  gpointer user_data)
{
	cairo_text_extents_t extents;
	GtkWidget *win = gtk_widget_get_toplevel(widget);	
	  
	
	cairo_set_source_rgb(cr, 0.1, 0.1, 0.1); 

  	cairo_select_font_face(cr, "Purisa",
      	CAIRO_FONT_SLANT_NORMAL,
      	CAIRO_FONT_WEIGHT_BOLD);

  	cairo_set_font_size(cr, 30);

  	cairo_text_extents(cr, "Hex Game - 4 x 4", &extents);
	gint w, h;
  	gtk_window_get_size(GTK_WINDOW(win), &w, &h);
  	cairo_move_to(cr, w/2 - extents.width/2, 40);  
  	cairo_show_text(cr, "Hex Game - 4 x 4");  
	int i,j,k;
	char num[] = "4";
	char let[] = "d";
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); 
	for(i=0;i<SIZE;++i) {
		cairo_move_to(cr, hexcell[0][0] - 51*i +525, hexcell[0][1] + h/4 + -30*i + 10); 
		cairo_show_text(cr, num);
		num[0] -= 1;
		cairo_move_to(cr, hexcell[0][0] + 51*i + 130, hexcell[0][1] + h/4 + -30*i + 15); 
		cairo_show_text(cr, let);
		let[0] -= 1;
	}
	for(i=0;i<7;++i){
		points[i][0] = hexcell[i][0] + 7*(w/15);
		points[i][1] = hexcell[i][1] + h/5;
        }
	for(i=0;i<SIZE;++i) {
		for(j=0;j<SIZE;++j) {
			switch(board[i][j]){
			case BLACK:
				cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
				break;
			case WHITE:
				cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
				break;
			default:	
		 		cairo_set_source_rgb(cr, 211./255., 211./255., 211./255.);
				break;	
			}
		  	for (k = 0; k < HEXAGON; k++) {
      				cairo_line_to(cr, points[k][0], points[k][1]);
  			}
		  	cairo_close_path(cr);
  			cairo_fill(cr);

		 	cairo_set_source_rgb(cr, 0.105, 0.105, 0.105);
		  	for (k = 0; k < HEXAGON; k++) {
      				cairo_line_to(cr, points[k][0], points[k][1]);
  			}
		  	cairo_close_path(cr);
  			cairo_stroke(cr);
			cairo_translate(cr, -51, 30);
		}
		cairo_translate(cr, 255, -90);
	}
	cairo_translate(cr,-255,-150);	
	cairo_set_font_size(cr, 30);
	if(isThereAWinner()) {
		cairo_text_extents(cr, player == WHITE ? "BLACK WINS" : "WHITE WINS", &extents);
	  	cairo_move_to(cr, w/4 - extents.width/2, 410);
	  	cairo_show_text(cr, player == WHITE ? "BLACK WINS" : "WHITE WINS");
	}
  	else {
		cairo_text_extents(cr, player == WHITE ? "WHITE PLAYS" : "BLACK PLAYS", &extents);
	  	cairo_move_to(cr, w/4 - extents.width/2, 410);
	  	cairo_show_text(cr, player == WHITE ? "WHITE PLAYS" : "BLACK PLAYS");
	}
	cairo_stroke(cr);
	cairo_set_line_width (cr, 7);
	cairo_line_to(cr,400,160);
	cairo_line_to(cr,298,220);
	cairo_stroke(cr);
	cairo_line_to(cr,510,385);
	cairo_line_to(cr,612,325);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_line_to(cr,510,160);
	cairo_line_to(cr,612,220);
	cairo_stroke(cr);
	cairo_line_to(cr,400,385);
	cairo_line_to(cr,298,325);
	cairo_stroke(cr);
  	return FALSE;
}

void generateBoard () {

	board = (int **) calloc (SIZE, sizeof(int *));
	boardPlayer1 = (int **) calloc (SIZE, sizeof(int *));
	groupsPlayer1 = (int *) calloc (SIZE*SIZE/2, sizeof(int));
	boardPlayer2 = (int **) calloc (SIZE, sizeof(int *));
	groupsPlayer2 = (int *) calloc (SIZE*SIZE/2, sizeof(int));
	int i;
	for (i = 0; i < SIZE; i++) {
		board[i] = (int *) calloc (SIZE, sizeof(int));
		boardPlayer1[i] = (int *) calloc (SIZE, sizeof(int));
		boardPlayer2[i] = (int *) calloc (SIZE, sizeof(int));
	}
}

int isThereAWinner () {
	int i;
	int j;
	for(i = 0; i < SIZE; i++)
		for(j = 0; j < SIZE; j++) {
			if((boardPlayer1[0][i] == boardPlayer1[SIZE-1][j]) && (boardPlayer1[0][i]*boardPlayer1[SIZE-1][j] != 0)) /* player 1 wins */
				return TRUE;
			if((boardPlayer2[i][0] == boardPlayer2[j][SIZE-1]) && (boardPlayer2[i][0]*boardPlayer2[j][SIZE-1] != 0)) /* player 2 wins */
				return TRUE;
		}
	return FALSE;
}

void updateB(int i, int j, int m, int n) {
	int k, l;
	int groupij = boardPlayer1[i][j];
	int groupmn = boardPlayer1[m][n];
	if(belong) {
		groupsPlayer1[groupmn-1] = 0;
		for(k = 0; k < SIZE; k++) {
			for(l = 0; l < SIZE; l++) {
				if(boardPlayer1[k][l] == groupmn)
					boardPlayer1[k][l] = groupij;
			}
		}
	} else {
		boardPlayer1[i][j] = groupmn;
		belong = TRUE;
	}
}

void updateW(int i, int j, int m, int n) {
	int k, l;
	int groupij = boardPlayer2[i][j];
	int groupmn = boardPlayer2[m][n];
	if(belong) {
		groupsPlayer2[groupmn-1] = 0;
		for(k = 0; k < SIZE; k++) {
			for(l = 0; l < SIZE; l++) {
				if(boardPlayer2[k][l] == groupmn)
					boardPlayer2[k][l] = groupij;
			}
		}
	} else {
		boardPlayer2[i][j] = groupmn;
		belong = TRUE;
	}
}

void compare(int i, int j, int m, int n) {
	if(board[i][j] != 0 && board[m][n] != 0) {
		if(board[i][j] == board[m][n]) {
			if(board[i][j] == 1 && (boardPlayer1[i][j] != boardPlayer1[m][n])) {
				updateB(i,j,m,n);
			}
			if(board[i][j] == -1 && (boardPlayer2[i][j] != boardPlayer2[m][n])) {	
				updateW(i,j,m,n);
			}
		}
	}
}

void checkBoard(int i, int j) {
	belong = FALSE;
	if(i-1 >= 0) {
		compare(i,j,i-1,j);
		if(j-1 >= 0)
			compare(i,j,i-1,j-1);
	}
	if (j-1 >= 0)
		compare(i,j,i,j-1);
	if(j+1 < SIZE)
		compare(i,j,i,j+1);
	if (i+1 < SIZE){
		compare(i,j,i+1,j);
		if(j+1 < SIZE)
			compare(i,j,i+1,j+1);
	}
	int k;
	if (!belong) {
		if(board[i][j] == 1) {
			for(k = 0; k < SIZE*SIZE/2; k++) {
				if(groupsPlayer1[k] == 0) {
					groupsPlayer1[k] = k+1;
					boardPlayer1[i][j] = k+1;
					break;
				}
			}
		}
		if(board[i][j] == -1) {
			for(k = 0; k < SIZE*SIZE/2; k++) {
				if(groupsPlayer2[k] == 0) {
					groupsPlayer2[k] = k+1;
					boardPlayer2[i][j] = k+1;
					break;
				}
			}
		}
	}
}

void first_player_strategy(void){
	if(turn == 1){
		board[2][2] = WHITE;
		checkBoard(2,2);
	}
	else if(turn == 3){
		if(board[2][3] == BLACK) {
			board[3][3] = WHITE;
			checkBoard(3,3);
		}
		else if(board[3][3] == BLACK){
			board[2][3] = WHITE;
			checkBoard(2,3);
		}
		else if(board[2][0] == BLACK || board[3][0] == BLACK || board[2][1] == BLACK || board[3][1] == BLACK || board[3][2] == BLACK) {
			board[1][1] = WHITE;
			checkBoard(1,1);
		}
		else {
			board[3][1] = WHITE;
			checkBoard(3,1);
		}
 	} else if(turn == 5) {
		if(board[2][3] && board[3][3]){
			if(board[2][0] == BLACK || board[3][0] == BLACK || board[2][1] == BLACK || board[3][1] == BLACK || board[3][2] == BLACK){
				board[1][1] = WHITE;
				checkBoard(1,1);
			}
			else {
				board[3][1] = WHITE;
				checkBoard(3,1);
			}
		} else if(board[2][3] == BLACK) {
			board[3][3] = WHITE;
			checkBoard(3,3);
		}
		else if(board[3][3] == BLACK){
			board[2][3] = WHITE;
			checkBoard(2,3);
		}
		else if(board[1][1] == WHITE){
			if(board[0][0] == BLACK){
				board[1][0] = WHITE;
				checkBoard(1,0);
			}
			else {
				board[0][0] = WHITE;
				checkBoard(0,0);
			}
		}
		else if(board[3][1] == WHITE){
			if(board[2][1] == BLACK){
				board[3][2] = WHITE;
				checkBoard(3,2);
			}
			else if(board[3][2] == BLACK){
				board[2][1] = WHITE;
				checkBoard(2,1);
			}
			else if(board[2][0] == BLACK){
				board[3][0] = WHITE;
				checkBoard(3,0);
			}
			else {
				board[2][0] = WHITE;
				checkBoard(2,0);
			}
		}
	} else if(turn == 7) {
		if(board[2][3] != EMPTY && board[3][3] != EMPTY){
			if(board[1][1] == WHITE){
				if(board[0][0] == EMPTY){
					board[0][0] = WHITE;
					checkBoard(0,0);
				}
				else if(board[1][0] == EMPTY){
					board[1][0] = WHITE;
					checkBoard(1,0);
				}
			} else if(board[3][1] == WHITE){
				if(board[2][1] == BLACK){
					board[3][2] = WHITE;
					checkBoard(3,2);
				}
				else if(board[3][2] == BLACK){
					board[2][1] = WHITE;
					checkBoard(2,1);
				}
				else if(board[2][0] == BLACK){
					board[3][0] = WHITE;
					checkBoard(3,0);
				}
				else if(board[3][0] == BLACK){ 	
					board[2][0] = WHITE;
					checkBoard(2,0);
				}	
			}
			
		} else if(board[2][3] == BLACK){
			board[3][3] = WHITE;
			checkBoard(3,3);
		}
		else if(board[3][3] == BLACK){
			board[2][3] = WHITE;
			checkBoard(2,3);
		}
		else if(board[1][1] == WHITE){
			if(board[0][0] == BLACK && board[1][0] == EMPTY){
				board[1][0] = WHITE;
				checkBoard(1,0);
			}
			else if(board[1][0] == BLACK && board[0][0] == EMPTY){
				board[0][0] = WHITE;
				checkBoard(0,0);
			}
			else{
				board[3][3] = WHITE;
				checkBoard(3,3);
			}
		} else if(board[3][1] == WHITE) {
				if(board[2][1] == BLACK && board[3][2] == EMPTY){
					board[3][2] = WHITE;
					checkBoard(3,2);
				}
				else if(board[3][2] == BLACK && board[2][1] == EMPTY){
					board[2][1] = WHITE;
					checkBoard(2,1);
				}
				else if(board[2][0] == BLACK && board[3][0] == EMPTY){
					board[3][0] = WHITE;
					checkBoard(3,0);
				}
				else if(board[3][0] == BLACK && board[2][0] == EMPTY){ 	
					board[2][0] = WHITE;
					checkBoard(2,0);
				}
				else if(board[2][3] == EMPTY && board[3][3] == EMPTY){ 	
					board[3][3] = WHITE;
					checkBoard(3,3);
				}
				else if(board[2][1] == EMPTY && board[3][2] == EMPTY){	
					board[3][2] = WHITE;
					checkBoard(3,2);
				}
				else if(board[2][0] == EMPTY && board[3][0] == EMPTY){	
					board[3][0] = WHITE;
					checkBoard(3,0);
				}
		}
		
	} else if(turn == 9){
		if(board[2][3] == BLACK && board[3][3] == EMPTY){ 	
			board[3][3] = WHITE;
			checkBoard(3,3);
		}
		else if(board[3][3] == BLACK && board[2][3] == EMPTY){ 	
			board[2][3] = WHITE;
			checkBoard(2,3);
		}
		else if(board[2][1] == BLACK && board[3][2] == EMPTY){	
			board[3][2] = WHITE;
			checkBoard(3,2);
		}
		else if(board[3][2] == BLACK && board[2][1] == EMPTY){	
			board[2][1] = WHITE;
			checkBoard(2,1);
		}
		else if(board[2][0] == BLACK && board[3][0] == EMPTY){	
			board[3][0] = WHITE;
			checkBoard(3,0);
		}
		else if(board[3][0] == BLACK && board[2][0] == EMPTY){	
			board[2][0] = WHITE;
			checkBoard(2,0);
		}
		else if(board[2][3] == EMPTY && board[3][3] == EMPTY){ 	
			board[3][3] = WHITE;
			checkBoard(3,3);
		}
		else if(board[2][1] == EMPTY && board[3][2] == EMPTY){	
			board[3][2] = WHITE;
			checkBoard(3,2);
		}
		else if(board[2][0] == EMPTY && board[3][0] == EMPTY){	
			board[3][0] = WHITE;
			checkBoard(3,0);
		}
	}		
}

int main (int argc, char *argv[]) {

	generateBoard();
	if(num_hum_players == 1){
		first_player_strategy();
		player = player == WHITE ? BLACK : WHITE;
	}
	GtkWidget *window;
  	GtkWidget *darea;
	
	GtkWidget *table;

  	GtkWidget *label1;
  	GtkWidget *label2;
  	
  	GtkWidget *entry1;
  	GtkWidget *entry2;

 	gtk_init(&argc, &argv);

  	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); 
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	gtk_widget_set_size_request(window, 800, 600);
  	gtk_window_set_title(GTK_WINDOW(window), "Hex Game");
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("hex.png"));
	
	GtkWidget * box = gtk_vbox_new(FALSE,0);
	GtkWidget * xbox = gtk_vbox_new(FALSE,0);
	GtkWidget * ybox = gtk_vbox_new(FALSE,0);
	GtkWidget * xybox = gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),box);

	GtkWidget *menubar = gtk_menu_bar_new();
  	GtkWidget *filemenu = gtk_menu_new();

  	GtkWidget *file = gtk_menu_item_new_with_label("File");
  	GtkWidget *quit = gtk_menu_item_new_with_label("Quit");
	GtkWidget *new_game = gtk_menu_item_new_with_label("New game");

  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new_game);
  	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
  	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 3);

  	darea = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(box),darea,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(xybox),xbox,TRUE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(xybox),ybox,TRUE,FALSE,0);
  	g_signal_connect(G_OBJECT(darea), "draw", 
      	G_CALLBACK(on_draw_event), NULL);  
  	g_signal_connect(window, "destroy",
      	G_CALLBACK(gtk_main_quit), NULL);
	GtkWidget *submit_button = gtk_button_new_with_label("Play");
	GtkWidget *rb[2][SIZE];
	if (num_hum_players != 0) {
		GtkWidget *rbx1 = gtk_radio_button_new_with_label(NULL,"A");
	   	GtkWidget *rbx2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rbx1), "B");
		GtkWidget *rbx3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rbx2), "C");
		GtkWidget *rbx4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rbx3), "D");
		gtk_box_pack_end(GTK_BOX(xbox), rbx4, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(xbox), rbx3, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(xbox), rbx2, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(xbox), rbx1, FALSE,FALSE,SIZE);
		rb[0][0] = rbx1;
		rb[0][1] = rbx2;
		rb[0][2] = rbx3;
		rb[0][3] = rbx4;
		GtkWidget *rby1= gtk_radio_button_new_with_label(NULL,"1");
	   	GtkWidget *rby2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rby1), "2");
		GtkWidget *rby3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rby2), "3");
		GtkWidget *rby4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rby3), "4");
		gtk_box_pack_end(GTK_BOX(ybox), rby4, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(ybox), rby3, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(ybox), rby2, FALSE,FALSE,SIZE);
		gtk_box_pack_end(GTK_BOX(ybox), rby1, FALSE,FALSE,SIZE);
	
		rb[1][0] = rby1;
		rb[1][1] = rby2;
		rb[1][2] = rby3;
		rb[1][3] = rby4;
		gtk_box_pack_end(GTK_BOX(box), xybox, FALSE,FALSE,SIZE);
	}
   	gtk_box_pack_end(GTK_BOX(box), submit_button, FALSE,FALSE,SIZE);
   	g_signal_connect(G_OBJECT(submit_button),
                     "clicked",
                     G_CALLBACK(clicked_call_back),
 		     (gpointer)rb);
	  
	



  	g_signal_connect(window, "destroy",
      	G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(new_game), "activate",
        G_CALLBACK(new_game_call_back), (gpointer)window);
  	g_signal_connect(G_OBJECT(quit), "activate",
        G_CALLBACK(gtk_main_quit), NULL);
	

  	
  	gtk_widget_show_all(window);

  	gtk_main();


	return 0;
}
