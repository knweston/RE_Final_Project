#pragma once
#include<vector>
#include<string>
#include<fstream>
using namespace std;

class Card
{
private:
	int id;
	int if_visible;
	int suit;
	int number;
public:
	Card(int card_id, int visible);
	int getID();
	string name();
};

class GameStats
{
private:
	int *hMem, *base;
	char *draw_base[2];
	char *target_base[4];
	char *tile_base[7];
	vector<Card> draw[2];
	vector<Card> target[4];
	vector<Card> tile[7];
public:
	void getInfo();
	void print();
};