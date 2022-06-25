#include "tetris.h"

static struct sigaction act, oact;

int main()
{
	int exit = 0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	createRankList(); // 함수 실행시 랭크 리스트 만들도록
	srand((unsigned int)time(NULL));

	while (!exit)
	{
		clear();
		switch (menu())
		{
		case MENU_PLAY:
			play();
			break;
		case MENU_RANK:
			rank();
			break;
		case MENU_REC_PLAY:
			recommendedPlay();
			break;
		case MENU_EXIT:
			exit = 1;
			break;
		default:
			break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris()
{
	int i, j;

	for (j = 0; j < HEIGHT; j++)
		for (i = 0; i < WIDTH; i++)
			field[j][i] = 0;

	for (i = 0; i < VISIBLE_BLOCKS; i++) // VISIBLE_BLOCK 수 만큼 블록 생성
	{
		nextBlock[i] = rand() % 7;
	}

	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	score = 0;
	gameOver = 0;
	timed_out = 0;

	// Tree Node(RecNode) 초기화
	root = (RecNode *)malloc(sizeof(RecNode));
	root->level = 0;
	root->accumulatedScore = 0;
	root->recBlockRotate = 0;
	root->recBlockY = 0;
	root->recBlockX = 0;
	for (j = 0; j < HEIGHT; j++)
		for (i = 0; i < WIDTH; i++)
			root->recField[j][i] = field[j][i];
	modified_recommend(root);

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline()
{
	int i, j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0, 0, HEIGHT, WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2, WIDTH + 10);
	printw("NEXT BLOCK");
	DrawBox(3, WIDTH + 10, 4, 8);
	DrawBox(9, WIDTH + 10, 4, 8); // 두번째 다음 블록그리는 공간

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15, WIDTH + 10); // 점수표 공간 아래로 더 내림 9->15
	printw("SCORE");
	DrawBox(16, WIDTH + 10, 1, 8);
}

int GetCommand()
{
	int command;
	command = wgetch(stdscr);
	switch (command)
	{
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ': /* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command)
{
	int ret = 1;
	int drawFlag = 0;
	switch (command)
	{
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if ((drawFlag = CheckToMove(field, nextBlock[0], (blockRotate + 1) % 4, blockY, blockX)))
			blockRotate = (blockRotate + 1) % 4;
		break;
	case KEY_DOWN:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX + 1)))
			blockX++;
		break;
	case KEY_LEFT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX - 1)))
			blockX--;
		break;
	default:
		break;
	}
	if (drawFlag)
		DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
	return ret;
}

void DrawField()
{
	int i, j;
	for (j = 0; j < HEIGHT; j++)
	{
		move(j + 1, 1);
		for (i = 0; i < WIDTH; i++)
		{
			if (field[j][i] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(".");
		}
	}
}

void PrintScore(int score)
{
	move(17, WIDTH + 11);
	printw("%8d", score);
}

void DrawNextBlock(int *nextBlock)
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		move(4 + i, WIDTH + 13);
		for (j = 0; j < 4; j++)
		{
			if (block[nextBlock[1]][0][i][j] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(" ");
		}
	}
	// 두번째 다음 블록을 그리는 과정
	for (i = 0; i < 4; i++)
	{
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++)
		{
			if (block[nextBlock[2]][0][i][j] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID, int blockRotate, char tile)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0)
			{
				move(i + y + 1, j + x + 1);
				attron(A_REVERSE);
				printw("%c", tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT, WIDTH + 10);
}

void DrawBox(int y, int x, int height, int width)
{
	int i, j;
	move(y, x);
	addch(ACS_ULCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for (j = 0; j < height; j++)
	{
		move(y + j + 1, x);
		addch(ACS_VLINE);
		move(y + j + 1, x + width + 1);
		addch(ACS_VLINE);
	}
	move(y + j + 1, x);
	addch(ACS_LLCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play()
{
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do
	{
		if (timed_out == 0)
		{
			alarm(1);
			timed_out = 1;
		}

		// recommendedPlay flag에 따라 받는 command 구분
		if (RecPlay == 1)
		{
			command = RecGetCommand();
		}
		else
		{
			command = GetCommand();
		}
		if (ProcessCommand(command) == QUIT)
		{
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu()
{
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX)
{
	// 움직일 수 있으면 0, 없으면 1 리턴
	int i, j, x, y;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (block[currentBlock][blockRotate][i][j] == 1)
			{
				y = blockY + i;
				x = blockX + j;
				if (!(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)) // field 범위 벗어나는지 확인
					return 0;
				else if (f[y][x] == 1) // field가 채워져있는지 확인
					return 0;
			}
		}
	}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX)
{
	// 1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	// 2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	// 3. 새로운 블록 정보를 그린다.
	int i, j;
	int oldRotate = blockRotate, oldY = blockY, oldX = blockX;
	int shadow_pos;
	switch (command) // 이전 블록에 대한 정보를 얻기 위한 switch문
	{
	case KEY_UP:
		oldRotate = (blockRotate + 3) % 4; // 이전 blockRotate를 얻어내기 위해서
		break;
	case KEY_DOWN:
		oldY = blockY - 1; // 아랫방향키 이전의 y좌표
		break;
	case KEY_RIGHT:
		oldX = blockX - 1; // 오른방향키 이전의 x좌표
		break;
	case KEY_LEFT:
		oldX = blockX + 1; // 왼방향키 이전의 x 좌표
		break;
	}
	shadow_pos = oldY;
	// 이전 그림자의 위치를 shadow_pos라는 변수에 저장
	while (CheckToMove(field, currentBlock, oldRotate, shadow_pos + 1, oldX) == 1 && shadow_pos + 1 < HEIGHT)
	{
		shadow_pos++;
	}

	for (i = 0; i < 4; i++) // 이중 loop로 얻은 이전 블록과 그림자 정보를 화면에서 지우기
	{
		for (j = 0; j < 4; j++)
		{
			if (block[currentBlock][oldRotate][i][j] == 1)
			{
				if (i + oldY >= 0)
				{
					move(i + oldY + 1, j + oldX + 1);
					printw(".");
				}
				if (i + shadow_pos >= 0)
				{
					move(i + shadow_pos + 1, j + oldX + 1);
					printw(".");
				}
			}
		}
	}
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
	move(HEIGHT, WIDTH + 10);
}

void BlockDown(int sig)
{
	//강의자료 p26-27의 플로우차트를 참고한다.
	int i, j;
	// recommendedPlay인 경우
	if (RecPlay == 1)
	{
		blockY = root->recBlockY;
		blockX = root->recBlockX;
		blockRotate = root->recBlockRotate;
	}
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX))
	{
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}

	else // 블록이 한칸 내려갈 수 없을 때
	{
		if (blockY == -1) // y 좌표가 -1이면 gameover
			gameOver = 1;

		// Field에 블록 합치고 완전히 채워진 width의 line은 제거
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX); // touched로 리턴한 값 score에 누적
		score += DeleteLine(field);

		// VISIBLE_BLOCKS의 수에 맞게 nextBlock 세팅 및 화면의 다음 블록 상자에 다음 블록 출력
		for (i = 0; i < VISIBLE_BLOCKS - 1; i++)
		{
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[VISIBLE_BLOCKS - 1] = rand() % 7; // 마지막 블록 ID 새로 생성
		DrawNextBlock(nextBlock);

		// InitTetris처럼 변수 초기화
		blockY = -1;
		blockX = WIDTH / 2 - 2;
		blockRotate = 0;

		// Tree Node(RecNode) 재초기화
		root = (RecNode *)malloc(sizeof(RecNode));
		root->level = 0;
		root->accumulatedScore = 0;
		root->recBlockRotate = 0;
		root->recBlockY = 0;
		root->recBlockX = 0;
		for (j = 0; j < HEIGHT; j++)
			for (i = 0; i < WIDTH; i++)
				root->recField[j][i] = field[j][i];
		modified_recommend(root);

		// 점수 출력 및 현재 블록 갱신하여 출력
		PrintScore(score);
		DrawField();
	}
	timed_out = 0; // 타이머 리셋
}

int AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX)
{
	// Block이 추가된 영역의 필드값을 바꾼다.
	int i, j, touched = 0;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (block[currentBlock][blockRotate][i][j] == 1)
			{
				f[blockY + i][blockX + j] = 1;
				// 바로 아래 필드가 1이고 HEIGHT, 즉 바닥에 닿았을 경우 혹은
				// 바로 아래 필드가 채워져 있는 경우 touched 증가
				if (blockY + i + 1 == HEIGHT || f[blockY + i + 1][blockX + j] == 1)
					touched++;
			}
		}
	}
	// touched 값으로 계산한 것 리턴하여 BlockDown에서 score에 누적시킴
	return touched * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH])
{
	// 1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	// 2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	int deleted = 0;
	int flag, i, j, y, x;
	for (i = 0; i < HEIGHT; i++)
	{
		flag = 1;
		for (j = 0; j < WIDTH; j++)
		{
			if (!(f[i][j]))
			{
				flag = 0;
				break;
			}
		}
		if (flag)
		{
			deleted++;
			for (y = i - 1; y >= 0; y--)
			{
				for (x = 0; x < WIDTH; x++)
				{
					f[y + 1][x] = f[y][x];
				}
			}
		}
	}
	return deleted * deleted * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID, int blockRotate)
{
	// 움직일 수 있을 때까지 y++로 간 다음
	while (CheckToMove(field, blockID, blockRotate, y, x) == 1 && y < HEIGHT)
	{
		y++;
	}
	// 움직일 수 없는 지점에서 y--로 이동한 뒤 '/'로 채워진 현재 블록 그려넣기
	y--;
	DrawBlock(y, x, blockID, blockRotate, '/');
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate)
{
	// 현재 블록과 그림자 그리기
	DrawBlock(y, x, blockID, blockRotate, ' ');
	DrawShadow(y, x, blockID, blockRotate);
	// 추천된 위치에 R이 그려진 블록 그리기
	DrawRecommend(root->recBlockY, root->recBlockX, blockID, root->recBlockRotate);
}

void createRankList()
{
	// 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
	// 1. "rank.txt"열기
	// 2. 파일에서 랭킹정보 읽어오기
	// 3. LinkedList로 저장
	// 4. 파일 닫기
	FILE *fp;
	int i, j, score = 0;
	char name[NAMELEN];
	Node *current;
	Node *NewNode;

	// 1. 파일 열기
	fp = fopen("rank.txt", "r");
	if (fp == NULL)
	{
		// rank.txt 파일 없을 때 생성하고 닫기
		fp = fopen("rank.txt", "w");
		fclose(fp);
		return;
	}
	// 2. 정보읽어오기
	/* int fscanf(FILE* stream, const char* format, ...);
	stream:데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
	// EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
	else
	{
		if (fscanf(fp, "%d\n", &score_number) != EOF)
		{
			for (i = 0; i < score_number; i++)
			{
				fscanf(fp, "%s %d\n", name, &score);
				NewNode = (Node *)malloc(sizeof(Node));
				strcpy(NewNode->name, name);
				NewNode->score = score;
				if (head == NULL)
				{
					head = NewNode;
					current = head;
				}
				else
				{
					current->link = NewNode;
					current = current->link;
				}
			}
		}
	}
	// 4. 파일닫기
	fclose(fp);
}

void rank()
{
	//목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
	// 1. 문자열 초기화
	int X = 1, Y = score_number, ch, i, count;
	clear();
	// 2. printw()로 3개의 메뉴출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	// 3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);
	// 4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	// 4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
	if (ch == '1')
	{
		echo();
		printw("X: ");
		scanw("%d", &X);
		printw("Y: ");
		scanw("%d", &Y);
		noecho();
		if (X == 0)
			X = 1;
		if (Y == 0 || Y > score_number)
			Y = score_number;
		printw("       name       |   score   \n");
		printw("------------------------------\n");
		if (X < 0 || Y < 0 || X > Y || X > score_number || score_number == 0)
		{
			printw("search failure: no rank in the list\n");
		}

		Node *current = head;
		for (count = 1; count <= Y; count++)
		{
			if (count >= X)
			{
				printw(" %-17s|   %d\n", current->name, current->score);
			}
			if (current->link != NULL)
			{
				current = current->link;
			}
		}
	}
	// 4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if (ch == '2')
	{
		char str[NAMELEN + 1];
		int check = 0;
		echo();
		printw("input the name: ");
		scanw("%s", str);
		noecho();
		Node *current = head;
		printw("       name       |   score   \n");
		printw("------------------------------\n");
		while (current != NULL) // current로 반복조건을 설정해야 마지막 Node에서도 비교하게 됨
		{
			if (!(strcmp(current->name, str))) //입력받은 문자와 각 Node의 이름을 비교
			{
				printw(" %-17s|   %d\n", current->name, current->score);
				check++;
			}
			current = current->link;
		}
		if (check == 0) // check가 0이면 같은 것이 없었던 것이므로
			printw("\nsearch failure: no name in the list\n");
	}

	// 4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if (ch == '3')
	{
		int num, i;
		echo();
		printw("input the rank: ");
		scanw("%d", &num);
		printw("\n");
		noecho();
		Node *prev = head;
		// score_number 안의 범위에 속할 때만 동작
		if (num > 0 && num <= score_number)
		{
			if (num == 1) // 첫 랭킹 지울 때
			{
				head = prev->link;
				free(prev);
			}
			else
			{
				for (i = 0; i < num - 2; i++) // 첫 랭킹 이외의 랭킹 지울 때
				{
					prev = prev->link;
				}
				Node *current = prev->link;
				prev->link = current->link;
				free(current);
			}
			score_number--; // 총 랭킹 개수 감소 업데이트
			printw("result: the rank deleted\n");
			writeRankFile(); // 총 랭킹 개수 감소 업데이트 바탕으로 rank.txt에 반영
		}
		// 범위 밖 예외처리
		else
			printw("search failure: the rank not in the list\n");
	}
	getch();
}

void writeRankFile()
{
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
	int i;
	Node *current = head;
	// 1. "rank.txt" 연다
	FILE *fp = fopen("rank.txt", "w");

	// 2. 랭킹 정보들의 수를 "rank.txt"에 기록
	fprintf(fp, "%d\n", score_number);

	// head가 NULL일 때까지 탐색한 다음 기록하도록 반복
	for (i = 0; i < score_number; i++)
	{
		fprintf(fp, "%s %d\n", current->name, current->score);
		if (current->link == NULL)
			break;
		current = current->link;
	}
	fclose(fp);
}

void newRank(int score)
{
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
	char str[NAMELEN + 1];
	clear();
	// 1. 사용자 이름을 입력받음
	printw("your name: ");
	echo();
	scanw("%s", str);
	noecho();
	// 2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	Node *NewNode = (Node *)malloc(sizeof(Node));
	Node *current;
	NewNode->score = score;
	strcpy(NewNode->name, str);
	NewNode->link = NULL;
	// 랭킹 개수가 0인 경우
	if (head == NULL)
	{
		head = NewNode;
	}
	// 랭킹 개수가 0이 아닌 경우
	else
	{
		current = head;
		if (current->score <= score)
		{
			NewNode->link = current;
			head = NewNode;
		}
		else
		{
			while (current->link != NULL)
			{
				if (current->score >= score && current->link->score < score)
				{
					NewNode->link = current->link;
					current->link = NewNode;
					break;
				}
				current = current->link;
			}
			current->link = NewNode;
		}
	}
	score_number++;
	writeRankFile();
}

void DrawRecommend(int y, int x, int blockID, int blockRotate)
{
	// user code
	DrawBlock(y, x, blockID, blockRotate, 'R'); // DrawShadow 함수 참고
}

//추천 Y좌표를 얻기위해서 새로 추가한 함수
int RecommendYpos(char field[HEIGHT][WIDTH], int y, int x, int blockID, int blockRotate)
{
	// DrawShadow 함수 참고
	while (CheckToMove(field, blockID, blockRotate, y, x) == 1 && y < HEIGHT)
	{
		y++;
	}
	y--;
	return y;
}

int recommend(RecNode *root)
{
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	// user code
	int Y, X, rotateNum, temp_score, c_index = 0;
	int i, j;
	RecNode **c = root->c;

	for (rotateNum = 0; rotateNum < 4; rotateNum++) // 블록 회전수에 대한 outer loop
		for (X = -2; X < WIDTH; X++)									// 놓일 수 있는 X좌표에 대한 loop
		{
			if (!(CheckToMove(root->recField, nextBlock[root->level], rotateNum, 0, X))) // 움직일 수 없으면 continue
				continue;
			else
			{
				temp_score = 0, Y = 0;
				// 움직일 수 있으면 children의 Node 생성 후 추천 과정 거침
				c[c_index] = (RecNode *)malloc(sizeof(RecNode));
				for (j = 0; j < HEIGHT; j++)
					for (i = 0; i < WIDTH; i++)
						c[c_index]->recField[j][i] = root->recField[j][i];
				c[c_index]->level = root->level + 1;
				// 추천 Y좌표 찾기
				Y = RecommendYpos(c[c_index]->recField, 0, X, nextBlock[root->level], rotateNum);
				// 점수 누적
				temp_score += AddBlockToField(c[c_index]->recField, nextBlock[root->level], rotateNum, Y, X);
				temp_score += DeleteLine(c[c_index]->recField);
				// 최대 고려 level 수보다 작은 경우 recursive 함수 호출로 점수 누적
				if (c[c_index]->level < VISIBLE_BLOCKS)
					temp_score += recommend(c[c_index]);
				// 현재까지의 최대 점수와 현재 얻어진 점수 비교하여 클 경우 정보 저장 및 점수 갱신
				if (max <= temp_score) // nextblock 3개를 쌓았을 때 가장 낮은 높이일 때로 개선하면 어떨지?? (조교님이 주신 아이디어)
				{
					root->recBlockY = Y;
					root->recBlockX = X;
					root->recBlockRotate = rotateNum;
					root->accumulatedScore = temp_score;
					max = temp_score;
				}
				c_index++; // children의 index 증가
			}
		}

	return max;
}

// 개선 1. 블록 별 가능한 회전수만큼 반복하도록 2. 쌓인 블록의 높이가 가장 낮게 되도록
int modified_recommend(RecNode *root)
{
	int max = 0;
	int Y, X, rotateNum, temp_score, c_index = 0;
	int possibleRotateNum[7] = {2, 4, 4, 4, 1, 2, 2};
	int i, j;
	RecNode **c = root->c;
	// 모든 블록의 회전수가 4가 아니므로, possibleRoatateNum의 index로 blockID에 맞는 회전 수 저장
	for (rotateNum = 0; rotateNum < possibleRotateNum[nextBlock[root->level]]; rotateNum++)
		for (X = -2; X < WIDTH; X++)
		{
			if (!(CheckToMove(root->recField, nextBlock[root->level], rotateNum, 0, X))) // 움직일 수 없으면 continue
				continue;
			else
			{
				temp_score = 0, Y = 0;
				// 움직일 수 있으면 children의 Node 생성 후 추천 과정 거침
				c[c_index] = (RecNode *)malloc(sizeof(RecNode));
				for (j = 0; j < HEIGHT; j++)
					for (i = 0; i < WIDTH; i++)
						c[c_index]->recField[j][i] = root->recField[j][i];
				c[c_index]->level = root->level + 1;
				// 추천 Y좌표 찾기
				Y = RecommendYpos(c[c_index]->recField, 0, X, nextBlock[root->level], rotateNum);
				// 점수 누적
				temp_score += AddBlockToField(c[c_index]->recField, nextBlock[root->level], rotateNum, Y, X);
				temp_score += DeleteLine(c[c_index]->recField);

				// 조교님이 주신 아이디어 - nextBlock을 쌓아서 가장 낮은 높이가 되도록 Y의 좌표가 내려갈수록 커지므로 그것을 선호하도록 점수계산에 Y좌표의 제곱을 더함
				temp_score += Y * Y;

				// 최대 고려 level 수보다 작은 경우 recursive 함수 호출로 점수 누적
				if (c[c_index]->level < VISIBLE_BLOCKS)
					temp_score += recommend(c[c_index]);
				// 현재까지의 최대 점수와 현재 얻어진 점수 비교하여 클 경우 정보 저장 및 점수 갱신
				if (max <= temp_score)
				{
					root->recBlockY = Y;
					root->recBlockX = X;
					root->recBlockRotate = rotateNum;
					root->accumulatedScore = temp_score;
					max = temp_score;
				}
				c_index++; // children의 index 증가
			}
		}

	return max;
}

// command 중 Q와 q를 제외한 나머지는 NOTHING으로 처리하는 GetCommand()를 변형한 함수
int RecGetCommand()
{
	int command;
	command = wgetch(stdscr);
	switch (command)
	{
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

void recommendedPlay()
{
	// user code
	RecPlay = 1; // tetris.h에 선언한 전역변수인 recommendedPlay의 flag
	play();			 // play() 함수 recommendedPlay의 flag에 따라서 수정함
	RecPlay = 0; // recommendedPlay의 flag 재초기화
}
