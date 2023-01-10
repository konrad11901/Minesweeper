#include "pch.h"
#include "GameState.h"

GameState::GameState() : remaining_mines(0), remaining_fields(0), game_status(GameStatus::Won) {}

GameState::GameState(int initial_mines, int initial_fields) : remaining_mines(initial_mines),
    remaining_fields(initial_fields - initial_mines), game_status(GameStatus::Init) {}

void GameState::OnFieldFlag() {
    remaining_mines--;

    CheckAndHandleVictory();
}

void GameState::OnFieldUnflag() {
    remaining_mines++;
}

void GameState::OnFieldDiscover(bool is_mined) {
    remaining_fields--;
    if (is_mined) {
        game_status = GameStatus::Lost;
    }
    else {
        CheckAndHandleVictory();
    }
}

void GameState::OnBoardGenerate() {
    game_status = GameStatus::Ongoing;
}

int GameState::GetRemainingMines() {
    return remaining_mines;
}

GameStatus GameState::GetGameStatus() {
    return game_status;
}

bool GameState::HasWon() {
    return remaining_fields == 0 && remaining_mines == 0;
}

void GameState::CheckAndHandleVictory() {
    if (!HasWon()) {
        return;
    }

    game_status = GameStatus::Won;
}
