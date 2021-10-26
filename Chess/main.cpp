#include <iostream>
using namespace std;

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
    #include <GL/glut.h>
#endif // GLUT Library
//Model
#include "Model.h"
//Chess Game
#include "Chess/Game.h"
//#include "Chess\Game.h"
#include "Chess/ChessPlayer.h"
//Window size and position
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_POS_X 50
#define WINDOW_POS_Y 50

#define BUTTON_X -100
#define BUTTON_Y -100
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 75

#define SLEEP_TIME_MILLISECONDS 2 /// 1000 = 1 second

/** Prototype **/
void keyFunction(unsigned char key, int x, int y);
void specialFunction(int key, int x, int y);

void newAITurn();


/**
    Variables for look at function
    eye     - position of eye
    center  - position of the center of view
    up      - up vertex
*/
GLfloat     eyeX = 2.0, eyeY = 0.0, eyeZ = -5.0,
            centerX = 0.0, centerY = 0.0, centerZ =0.0,
            upX = 0.0, upY = 0.0, upZ = -1.0;

/**
    Variables for perspective function
*/
GLfloat     fovy = 50.0f, zNear = 0.1f, zFar = 20.0f;

/**
    Variables for light
*/
GLfloat     position[] = {0.0f, 0.0f, 100.0f, 0.0f};
GLfloat     diffusion[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat     normal_board[] = {0.0f, 0.0f, 1.0f};
GLfloat     normal_valid_move[] = {0.0f, 0.0f, -1.0f};
float       ang = 0;
GLfloat mat_diffusion[] = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat mat_specular[] = {0.1f, 0.1f, 0.1f, 1.0f};

/**
    Variables for managing view
*/
GLfloat     screen_ratio, zoomOut = 2;

/**
    Model Loading
*/
Model   Pawn("model/cube.obj");
Model   Rook("model/cube.obj");
Model   Knight("model/cube.obj");
Model   Bishop("model/cube.obj");
Model   King("model/cube.obj");
Model   Queen("model/cube.obj");

/**
    Pre-start
*/
bool pressed = false;

/**
    Game Loading
*/
Game *chess;
//Board* boardCopy = nullptr;

void newGame();

/**
    Real-time variables
*/
bool    inGame = false, verify = false;
int     selectedRow = 1, selectedCol = 1;
int     moveToRow = 1, moveToCol = 1;
bool    selected = false;
bool    board_rotating = true;
bool    ai_moving = false;
int     rotation = 0;
bool    check = false, checkMate = false;
bool    closeGame = false;
bool	needPromote = false;

Move    interpretedMove; // for user input

/**
    Chess board vertices
*/
GLfloat     chessBoard[12][3] = {{-4.0, -4.0, 0.5},
                                {-4.0,  4.0, 0.5},
                                { 4.0,  4.0, 0.5},
                                { 4.0, -4.0, 0.5},

                                {-4.5, -4.5, 0.5},
                                {-4.5,  4.5, 0.5},
                                { 4.5,  4.5, 0.5},
                                { 4.5, -4.5, 0.5},

                                {-5.0, -5.0, 0.0},
                                {-5.0,  5.0, 0.0},
                                { 5.0,  5.0, 0.0},
                                { 5.0, -5.0, 0.0}};


/**
*/
void showWord( int x, int y, string word)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH/2, WINDOW_WIDTH/2, -WINDOW_HEIGHT/2, WINDOW_HEIGHT/2, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    int l,i;

    l=word.length(); // see how many characters are in text string.
    glRasterPos2i(x, y); // location to start printing text
    glColor3f(1,1,0);
    for( i=0; i < l; i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, word[i]); // Print a character on the screen
    }
}

/**
*/
void drawMoveToSquare()
{
    float r = 1.0f*(moveToRow- (Board::HEIGHT / 2)), c = 1.0f*(moveToCol- (Board::WIDTH / 2));
    if(selected)
    {
        glPushMatrix();
            glColor3f(0.5f, 1.0f, 0.0f);
            glTranslatef(r, c, 0.502f);
            glScalef(0.98f, 0.98f, 1.0f);
                glBegin(GL_TRIANGLES);
                    glNormal3fv(normal_valid_move);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(1.0f, 0.0f, 0.0f);
                glEnd();
        glPopMatrix();
    }
    glColor3f(0, 0, 0);
}

/**
    Drawing a chess board using points from array "chessBoard"
*/
void drawChessBoard()
{
	//todo remove
	return;

    glPushMatrix();
        /**Drawing bottom of the chess board*/
        glNormal3fv(normal_valid_move);
        glBegin(GL_QUADS);
            glColor3f(1.0, 0.0, 0.0);
            for(int i=8; i<12; i++) glVertex3fv(chessBoard[i]);
        glEnd();
        /**Drawing top of the chess board*/
        glBegin(GL_QUADS);
            glColor3f(0.55f, 0.24f, 0.09f);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[0]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[4]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[5]);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[1]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[1]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[5]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[6]);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[2]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[2]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[6]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[7]);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[3]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[3]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[7]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[4]);
            glColor3f(0.803f, 0.522f, 0.247f);
            glVertex3fv(chessBoard[0]);
        glEnd();
        /**Drawing side of the chess board*/
        glBegin(GL_QUADS);
            glColor3f(1.0f, 0.95f, 0.9f);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[4]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[8]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[9]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[5]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[5]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[9]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[10]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[6]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[6]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[10]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[11]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[7]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[7]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[11]);
            glColor3f(1.000f, 1.000f, 1.000f);
            glVertex3fv(chessBoard[8]);
            glColor3f(0.545f, 0.271f, 0.075f);
            glVertex3fv(chessBoard[4]);
        glEnd();
    glPopMatrix();
    glColor3f(0, 0, 0);
}

void drawBoardSquares()
{
	if (chess == nullptr)
		return;

    const Board& board = chess->GetChessBoard();

    float r, c;
    for(int row = Board::MIN_ROW_INDEX ; row < Board::MAX_ROW_INDEX ; row++)
    {
        for(int col = Board::MIN_COL_INDEX ; col < Board::MAX_COL_INDEX ; col++)
        {
            r = 1.0f*(row- (Board::HEIGHT / 2));
            c = 1.0f*(col- (Board::WIDTH / 2));
            if(row == selectedRow && col == selectedCol)
            {
                unsigned int index = ToIndex(selectedCol, selectedRow);

                if(selected) glColor3f(0.33f, 0.420f, 0.184f);
                else if(IsValid(board[index]))
                    if(IsWhite(board[index]) == chess->IsWhiteTurn())
                        glColor3f(0.0f, 0.5f, 0.0f);
                    else glColor3f(1.0f, 0.0f, 0.0f);
                else glColor3f(0.3f, 0.7f, 0.5f);
            }
            else if((row + col) & 1) glColor3f(1.0, 1.0, 1.0);
            else glColor3f(0.0, 0.0, 0.0);
            glPushMatrix();
                glTranslatef(r, c, 0.5f);
                glBegin(GL_TRIANGLES);
                    glNormal3fv(normal_board);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(1.0f, 0.0f, 0.0f);
                glEnd();
            glPopMatrix();
        }
    }
    glColor3f(0, 0, 0);
}

/**
    Draw Valid Moves of Selected Piece
*/
void drawValidMoves()
{
	if (chess == nullptr)
		return;

    if(selected)
    {
        std::vector<Move> moves = chess->GetLegalmoves(selectedCol, selectedRow);
        int vec_size = moves.size(), row, col;

        for(int id = 0; id < vec_size; id++)
        {
            unsigned int endPosition = GetEndPos(moves[id]);
            row = endPosition >> 3;
            col = endPosition & 7;

            switch(GetMoveType(moves[id]))
            {
                case MoveType::NORMAL:
                    glColor3f(0.8f, 1.0f, 0.6f);
                    break;
                case MoveType::ADVANCED_PAWN:
                    glColor3f(0.8f, 1.0f, 0.6f);
                    break;
                case MoveType::PROMOTION:
                    glColor3f(1.0f, 0.5f, 0.6f);
                    break;
               // case MoveType::CAPTURE:
                    //glColor3f(1.0f, 0.0f, 0.0f);
                    //break;
                case MoveType::ENPASSANT_HIGHER:
                case MoveType::ENPASSANT_LOWER:
                    glColor3f(0.8f, 1.0f, 0.6f);
                    break;
                case MoveType::CASTLE_HIGHER:
                case MoveType::CASTLE_LOWER:
                    glColor3f(0.196f, 0.804f, 0.196f);
                    break;
            }

            float r = 1.0f*(row- (Board::HEIGHT / 2)), c = 1.0f*(col- (Board::WIDTH / 2));
            glPushMatrix();
                    glTranslatef(r, c, 0.501f);
                    glScalef(0.99f, 0.99f, 1.0f);
                    glBegin(GL_TRIANGLES);
                        glNormal3fv(normal_valid_move);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(1.0f, 1.0f, 0.0f);
                        glVertex3f(0.0f, 1.0f, 0.0f);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(1.0f, 1.0f, 0.0f);
                        glVertex3f(1.0f, 0.0f, 0.0f);
                    glEnd();
            glPopMatrix();
        }
    }
    glColor3f(0, 0, 0);
}

/**
    Drawing Chess Pieces in Board
*/
void drawChessPieces()
{
	if (chess == nullptr)
		return;

	//todo draw more

    const Board& board = chess->GetChessBoard();

    float z;
    for(int row = Board::MIN_ROW_INDEX; row < Board::MAX_ROW_INDEX; row++)
    {
        for(int col = Board::MIN_COL_INDEX; col < Board::MAX_COL_INDEX; col++)
        {
            if(IsValid(board.C_At(col, row)))
            {
                const bool isWhite = IsWhite(board.C_At(col, row));
                const PieceType pieceType = GetType(board.C_At(col, row));

                glPushMatrix();
                    if(selected && row == selectedRow && col == selectedCol) z = 1.0;
                    else z = 0.5;
                    glTranslatef((row - (Board::WIDTH / 2)) * 1.0f + 0.5f, (col - (Board::HEIGHT / 2)) * 1.0f + 0.5f, z);
                    glScalef(0.01f, 0.01f, 0.01f);

                    if (isWhite)
                    {
                        glRotatef(90, 0.0f, 0.0f, 1.0f);
                        glColor3f(1.0f, 1.0f, 0.5f);
                    }
                    else 
                    {
                        glRotatef(-90, 0.0f, 0.0f, 1.0f);
                        glColor3f(0.5f, 0.5f, 0.5f);
                    }

                    switch(pieceType)
                    {
					case PieceType::PAWN: Pawn.Draw(isWhite); break;
                        case PieceType::ROOK: Rook.Draw(isWhite); break;
                        case PieceType::KNIGHT: Knight.Draw(isWhite); break;
                        case PieceType::BISHOP: Bishop.Draw(isWhite); break;
                        case PieceType::QUEEN: Queen.Draw(isWhite); break;
                        case PieceType::KING: King.Draw(isWhite); break;
                    }
                glPopMatrix();
            }
            
        }
    }
    glColor3f(0, 0, 0);
}

void key_W_pressed(bool white)
{
    if (white) 
    {
        if (!selected && selectedRow < Board::MAX_ROW_INDEX - 1) selectedRow++;
        if (selected && moveToRow < Board::MAX_ROW_INDEX - 1) moveToRow++;
    }
    else 
    {
        if (!selected && selectedRow > Board::MIN_ROW_INDEX) selectedRow--;
        if (selected && moveToRow > Board::MIN_ROW_INDEX) moveToRow--;
    }
}

void key_D_pressed(bool white)
{
    if (white)
    {
        if (!selected && selectedCol < Board::MAX_COL_INDEX - 1) selectedCol++;
        if (selected && moveToCol < Board::MAX_COL_INDEX - 1) moveToCol++;
    }
    else
    {
        if (!selected && selectedCol > Board::MIN_COL_INDEX) selectedCol--;
        if (selected && moveToCol > Board::MIN_COL_INDEX) moveToCol--;
    }
}

void key_S_pressed(bool white)
{
    if (white)
    {
        if (!selected && selectedRow > Board::MIN_ROW_INDEX) selectedRow--;
        if (selected && moveToRow > Board::MIN_ROW_INDEX) moveToRow--;
    }
    else 
    {
        if (!selected && selectedRow < Board::MAX_ROW_INDEX - 1) selectedRow++;
        if (selected && moveToRow < Board::MAX_ROW_INDEX - 1) moveToRow++;
    }
}

void key_A_pressed(bool white)
{
    if (white)
    {
        if (!selected && selectedCol > Board::MIN_COL_INDEX) selectedCol--;
        if (selected && moveToCol > Board::MIN_COL_INDEX) moveToCol--;
    }
    else 
    {
        if (!selected && selectedCol < Board::MAX_COL_INDEX - 1) selectedCol++;
        if (selected && moveToCol < Board::MAX_COL_INDEX - 1) moveToCol++;
    }
}

void updateTurn(bool white)
{
    if (white)
    {
        selectedRow = Board::MIN_ROW_INDEX;
        selectedCol = Board::MAX_COL_INDEX - 1;
    }
    else 
    {
        selectedRow = Board::MAX_ROW_INDEX - 1;
        selectedCol = Board::MIN_COL_INDEX;
    }
}

#define BOARD_ROTATION 0 //4

void doRotationBoard(bool white)
{
	board_rotating = false;

    if (white)
    {
        if (rotation < 180) {
            rotation += BOARD_ROTATION;
        }
        else {
            board_rotating = false;

        }
    }
    else
    {
        if (rotation < 360) {
            rotation += BOARD_ROTATION;
        }
        else {
            rotation = 0;
            board_rotating = false;

        }
    }
}

void endOfTurn()
{
    selected = false;
    needPromote = false;
	check = false;

	//chess->nextTurn();
	if(chess->IsCheckMate())
	{
		checkMate = true;
	}
	else if(chess->IsCheck())
	{
		check = true;
	}
    
	board_rotating = true;
	updateTurn(chess->IsWhiteTurn());
}

void newAITurn()
{
	ChessPlayer* player = nullptr;
	if (chess->IsWhiteTurn()) {
		player = chess->GetWhitePlayer();
	}
	else {
		player = chess->GetBlackPlayer();
	}
	
	if (!player->IsAI() || chess->IsCheckMate()) 
		return;

	// decide and make the move
	Move move;
	bool moveMade = player->chooseAIMove(move);
	if (!moveMade)
	{
		return; // TODO: if this is the case - what next? The chess engine should have detected a checkmate prior to this. 
	}

    chess->MakePlayerMove(move);
    endOfTurn();
	Sleep(SLEEP_TIME_MILLISECONDS);

    unsigned int startPosition = GetStartPos(move);
    unsigned int endPosition = GetEndPos(move);

    selectedRow = (startPosition >> 3);
    selectedCol = (startPosition & 7);
	keyFunction(' ', 0, 0);

	moveToRow = (endPosition >> 3);
	moveToCol = (endPosition & 7);
	keyFunction(' ', 0, 0);

	ai_moving = true;
}

void displayFunction()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(inGame)
    {
        /**
            Changing view perspective
        */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fovy, screen_ratio, zNear, zoomOut * zFar);
        /**
            Drawing model mode
        */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(zoomOut * eyeX, zoomOut * eyeY, zoomOut * eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

        /**
            Draw code here
        */
		if (board_rotating) {

			doRotationBoard(chess->IsWhiteTurn());

			// take this as a signal to update the AI (if any)
			if(board_rotating == false) // a change of state - do the AI
				newAITurn();
		}
			

        GLfloat ambient_model[] = {0.5, 0.5, 0.5, 1.0};

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_model);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_diffusion);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusion);

        glScalef(1.0f, 1.0f, -1.0f);

        glLightfv(GL_LIGHT0, GL_POSITION, position);

        glRotatef((float)rotation, 0, 0, 1);

		
        drawChessBoard();

        drawBoardSquares();

		if(chess != nullptr) {
			drawChessPieces();

			drawMoveToSquare();

			drawValidMoves();

            //showWord(-300, WINDOW_HEIGHT / 2 - 24, "Material advantage: ");

			if(needPromote)
			{
				showWord(-200, WINDOW_HEIGHT/2-24, "Promote to: (Q) Queen | (R) Rook | (B) Bishop | (K) Knight");
			}
			else if(verify) 
				showWord(-200, WINDOW_HEIGHT/2-24, "Are you sure to retry? Yes (O)  or  No (X)");
			else
			{
				if(check)
				{
					string s = chess->IsWhiteTurn() == false ? "BLACK PIECE" : "WHITE PIECE";
					showWord(-150, WINDOW_HEIGHT/2-24, s+" CHECKED!");
				}
				if(checkMate)
				{
					string s = chess->IsWhiteTurn() ? "BLACK PLAYER" : "WHITE PLAYER";
					if (chess->IsCheck())
					{
						showWord(-100, WINDOW_HEIGHT / 2 - 24, "CHECK MATE!");
						showWord(-140, WINDOW_HEIGHT / 2 - 50, s + " WIN!");
						showWord(-150, -WINDOW_HEIGHT / 2 + 50, "Do you want to play again?");
						showWord(-120, -WINDOW_HEIGHT / 2 + 25, "Yes (O)  or  No (X)");
					}
					else
					{
						showWord(-100, WINDOW_HEIGHT / 2 - 24, "STALE MATE!");
						showWord(-140, WINDOW_HEIGHT / 2 - 50, " DRAW!");
						showWord(-150, -WINDOW_HEIGHT / 2 + 50, "Do you want to play again?");
						showWord(-120, -WINDOW_HEIGHT / 2 + 25, "Yes (O)  or  No (X)");
					}
				}
			}
		}
    }
    else
    {
        showWord(-150, 0, "- - Press N to Start The Game - -");
    }

	if(closeGame)
		glutExit();

    glutSwapBuffers();

    glutPostRedisplay();

	if (ai_moving) {
		ai_moving = false;
		Sleep(SLEEP_TIME_MILLISECONDS);
	}
}

void reshapeFunction(int width, int height)
{
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
    screen_ratio = (GLfloat) width / (GLfloat) height;
}

void specialFunction(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            zoomOut += 0.1f;
            break;
        case GLUT_KEY_DOWN:
            zoomOut -= 0.1f;
            break;
        case GLUT_KEY_LEFT:
            ang += 5;
            break;
        case GLUT_KEY_RIGHT:
            ang -= 5;
            break;
        default: break;
    }
}

void keyFunction(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'w':
        case 'W':
            if(!needPromote && !checkMate && !verify && inGame && !board_rotating) key_W_pressed(chess->IsWhiteTurn());
            break;
        case 'a':
        case 'A':
            if(!needPromote && !checkMate && !verify && inGame && !board_rotating) key_A_pressed(chess->IsWhiteTurn());
            break;
        case 's':
        case 'S':
            if(!needPromote && !checkMate && !verify && inGame && !board_rotating) key_S_pressed(chess->IsWhiteTurn());
            break;
        case 'd':
        case 'D':
            if(!needPromote && !checkMate && !verify && inGame && !board_rotating) key_D_pressed(chess->IsWhiteTurn());
            break;
        case ' ':
            if (!needPromote && !checkMate && !verify && inGame && !board_rotating)
            {
                
                const Board& board = chess->GetChessBoard();
                if (selected)
                {
                    // the selected piece is being moved
                    if (chess->InterpretMove(interpretedMove, selectedCol, selectedRow, moveToCol, moveToRow))
                    {
                        if (GetMoveType(interpretedMove) == MoveType::PROMOTION)
                        {
                            needPromote = true;
                        }

                        if(!needPromote)
                        {
                            chess->MakePlayerMove(interpretedMove);
                            endOfTurn();
                        }
                    }

                    selected = false;
                }
                else if (IsValid(board.C_At(selectedCol, selectedRow)) && IsWhite(board.C_At(selectedCol, selectedRow)) == chess->IsWhiteTurn())
                {
                    // a piece is being selected 
                    selected = !selected;
                    if (selected)
                    {
                        moveToRow = selectedRow;
                        moveToCol = selectedCol;
                    }
                }
                
            }
            break;
        case 'n':
        case 'N':
            if(!inGame)newGame();
            else verify = true;
            break;
        case 'o': case 'O':
            if(checkMate || verify) {delete chess; newGame(); verify = false;}
            break;
        case 'x': case 'X':
			if(checkMate) 
			{ 
				closeGame = true; 
				delete chess;
				chess = nullptr;
			}
            if(verify) 
			{
				verify = false;
			}
            break;
		case 'q': case 'Q':
			if(needPromote)
			{
                interpretedMove = SetPromoteMove(interpretedMove, PieceType::QUEEN);
                chess->MakePlayerMove(interpretedMove);
                endOfTurn();
				break;
			}
			else break;
		case 'r': case 'R':
			if(needPromote)
			{
                interpretedMove = SetPromoteMove(interpretedMove, PieceType::ROOK);
                chess->MakePlayerMove(interpretedMove);
                endOfTurn();
				break;
			}
			else break;
		case 'b': case 'B':
			if(needPromote)
			{
                interpretedMove = SetPromoteMove(interpretedMove, PieceType::BISHOP);
                chess->MakePlayerMove(interpretedMove);
                endOfTurn();
				break;
			}
			else break;
		case 'k': case 'K':
			if(needPromote)
			{
                interpretedMove = SetPromoteMove(interpretedMove, PieceType::KNIGHT);
                chess->MakePlayerMove(interpretedMove);
                endOfTurn();
				break;
			}
			else break;

        case 'z':

            chess->Undo();
            break;
		case 'c': case 'C':
			//boardCopy = chess->getCopyOfBoard();
			break;
		case 'v': case 'V':
			//if(boardCopy != nullptr)
				//chess->replaceBoard(boardCopy);
			break;

		default: break;
    }
}

void initialize()
{
	Pawn.Initialise("model/pawn_black.jpg", "model/pawn_white.jpg");
	Rook.Initialise("model/rook_black.jpg", "model/rook_white.jpg");
	Knight.Initialise("model/knight_black.jpg", "model/knight_white.jpg");
	Bishop.Initialise("model/bishop_black.jpg", "model/bishop_white.jpg");
	King.Initialise("model/king_black.jpg", "model/king_white.jpg");
	Queen.Initialise("model/queen_black.jpg", "model/queen_white.jpg");

    glClearColor(0.2f, 0.6f, 0.5f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SMOOTH);
    glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void newGame()
{
    chess = new Game();
    selectedRow = 1; selectedCol = 1;
    moveToRow = 1; moveToCol = 1;
    selected = false;
    board_rotating = true;
    rotation = 180;
    inGame = true;
    check = false;
    checkMate = false;
    updateTurn(chess->IsWhiteTurn());
}

int main(int argc, char *argv[])
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
    glutCreateWindow("Chess");

    initialize();
    glutDisplayFunc(displayFunction);
    glutReshapeFunc(reshapeFunction);
    glutKeyboardFunc(keyFunction);
    glutSpecialFunc(specialFunction);
    glutMainLoop();
	return 0;
}
