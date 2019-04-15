// Solver.cpp : Defines the exported functions for the DLL application.
//

//#include "stdafx.h"

#include "Solver.h"

Card::Card(int card_id, int visible)
{
	this->id = card_id;
	this->if_visible = visible;
	this->suit = card_id % 4;
	this->number = card_id / 4;
}

int Card::getID()
{
	return this->id;
}

string Card::name()
{
	string suit_name[] = { "Club", "Diamond", "Heart", "Spade" };
	string number_name[] = { "01","02","03","04","05","06","07","08","09","10","JA","QE","KG" };
	string visible;
	if (if_visible)visible = "V";
	else visible = "I";
	string result = "";
	result += suit_name[this->suit] + "_" + number_name[this->number] + "/" + visible;
	return result;
}

void GameStats::getInfo()
{
	int num_draw[2], num_target[4], num_tile[7];
	hMem = (int *)0x1007170;
	base = (int *)((int)*hMem + 0x6c);
	int decknum[] = { 2,4,7 }, deckoffset[] = { 0,2,6 };
	char **deckbase[] = { draw_base, target_base, tile_base };
	int *num_cards[] = { num_draw,num_target,num_tile };
	vector<Card> *vec_cards[] = { draw,target,tile };
	for (int k = 0; k < 3; k++)
	{
		for (int i = 0; i < decknum[k]; i++)
		{
			deckbase[k][i] = (char *)*(base + deckoffset[k] + i);
			num_cards[k][i] = *(int *)(deckbase[k][i] + 0x1c);
		}
		for (int i = 0; i < decknum[k]; i++)
		{
			for (int j = 0; j < num_cards[k][i]; j++)
			{
				char *content = (char *)(0x24 + j * 0xc + deckbase[k][i]);
				int card_id = *content, card_visible = *(content + 1);
				vec_cards[k][i].push_back(Card(card_id, card_visible));
			}
		}
	}
}

void GameStats::print()
{
	fstream fs;
	fs.open("out.txt", ios_base::out);

	fs << endl << "==================================================================================================================" << endl;
	fs << "MEMORY ADDRESSES INFO:";
	fs << endl << "==================================================================================================================" << endl;
	fs << "hMem: " << (void *)*hMem << endl;
	fs << "Draw deck address:" << endl;
	for (int i = 0; i < 2; i++)
		fs << (void *)draw_base[i] << "\t";
	fs << endl;
	fs << "Target deck address:" << endl;
	for (int i = 0; i < 4; i++)
		fs << (void *)target_base[i] << "\t";
	fs << endl;
	fs << "Tile addresses:" << endl;
	for (int i = 0; i < 7; i++)
		fs << (void *)tile_base[i] << "\t";

	fs << endl;
	fs << endl << "==================================================================================================================" << endl;
	fs << "DRAWING DECK:";
	fs << endl << "==================================================================================================================" << endl;
	for (int i = 0; i < draw[0].size(); i++)
	{
		fs << draw[0][i].getID() << "," << draw[0][i].name() << "\t";
		if (i % 6 == 5) fs << endl;
	}

	fs << endl;
	fs << endl << "==================================================================================================================" << endl;
	fs << "DRAWN CARDS:";
	fs << endl << "==================================================================================================================" << endl;
	for (int i = 0; i < draw[1].size(); i++)
	{
		fs << draw[1][i].getID() << "," << draw[1][i].name() << "\t";
		if (i % 6 == 5) fs << endl;
	}

	fs << endl;
	fs << endl << "==================================================================================================================" << endl;
	fs << "TARGET DECKS:";
	fs << endl << "==================================================================================================================" << endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < target[i].size(); j++)
		{
			fs << target[i][j].getID() << "," << target[i][j].name() << "\t";
		}
		fs << endl;
	}

	fs << endl;
	fs << endl << "==================================================================================================================" << endl;
	fs << "CARDS IN PLAY:";
	fs << endl << "==================================================================================================================" << endl;
	bool end[7] = { false };
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (i >= tile[j].size())
				fs << "\t\t";
			else 
				fs << tile[j][i].getID() << "," << tile[j][i].name() << "\t";
		}
		fs << endl;
	}
	fs.close();
}