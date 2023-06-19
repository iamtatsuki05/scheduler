#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESS_NUM 4       // プロセス数の定義。今回は4つのプロセスを想定。
#define TIME_QUANTUM 2      // タイムクォンタムの定義。タイムクォンタムはCPUが一度に処理できる時間単位。
#define Q_SIZE 10000        // キューのサイズを定義。キューはプロセスが待機する領域。

// プロセスの情報を保持する構造体
typedef struct Process{
    char name[10];      // プロセスの名前
    int arrival_time;   // プロセスがシステムに到着する時間
    int process_time;   // プロセスが処理に必要な時間
    int finish_time;    // プロセスが終了する時間
    int remaining_time; // プロセスが終了するまでに必要な残り時間
    int status;         // プロセスの状態を表すステータス (0: not in queue, 1: in queue, 2: finished execution)
} Process;

// プロセスのリストを初期化。今回は4つのプロセスA, B, C, Dが存在。
Process processList[PROCESS_NUM] = {
    {"A", 0, 2, 0, 2, 0},
    {"B", 1, 10, 0, 10, 0},
    {"C", 3, 7, 0, 7, 0},
    {"D", 5, 3, 0, 3, 0},
};

// プロセスリストのバックアップを保持。これはオリジナルのプロセスリストを変更する際に使用する。
Process backupProcessList[PROCESS_NUM];

// プロセスを待機させるためのキューを定義。キューは先入れ先出しのデータ構造。
Process *queue[Q_SIZE];
int queuehead = 0;     // キューの先頭を表すポインタ
int queuetail = 0;     // キューの末尾を表すポインタ

// utils
// プロセスをキューに追加する関数
void enqueue(Process *x){
    queuetail = (queuetail + 1) % Q_SIZE; // キューの末尾を1つ進める。キューのサイズを超えた場合は0に戻る。
    queue[queuetail] = x;                 // プロセスをキューの末尾に追加する
    if (queuehead == queuetail)           // キューがオーバーフローした場合のエラー処理
    {
        printf("Error: Overflow\n");
        return;
    }
}

// キューからプロセスを取り出す関数
Process *dequeue(){
    if (queuehead == queuetail)           // キューが空（先頭と末尾が同じ）の場合はNULLを返す
        return NULL;
    queuehead = (queuehead + 1) % Q_SIZE; // キューの先頭を1つ進める。キューのサイズを超えた場合は0に戻る。
    return queue[queuehead];              // キューの先頭のプロセスを返す
}

// 現在の時間に到着したタスクをキューに追加する関数
void fetch_new_task(Process *processList, int num_of_tasks, int time){
    // プロセスリスト内のすべてのタスクを走査
    for (int i = 0; i < num_of_tasks; ++i){
        // タスクが既に到着しており、まだキューに追加されていない場合
        if (processList[i].arrival_time <= time && !processList[i].status){
            enqueue(&processList[i]);   // タスクをキューに追加
            processList[i].status = 1;  // タスクのステータスを1（キュー内）に更新
        }
        // タスクがまだ到着していない場合、ループを抜ける（プロセスリストは到着時間でソートされていると仮定）
        if (processList[i].arrival_time > time)
            break;
    }
}

// 到着時間に基づいてプロセスを比較する関数
int compareByArrivalTime(const void *a, const void *b){
    Process *p1 = (Process *)a;  // aをProcess型へキャスト
    Process *p2 = (Process *)b;  // bをProcess型へキャスト

    return p1->arrival_time - p2->arrival_time;  // 到着時間に基づく差分を返す
}

// 処理時間に基づいてプロセスを比較する関数
int compareByProcessTime(const void *a, const void *b){
    Process *p1 = (Process *)a;  // aをProcess型へキャスト
    Process *p2 = (Process *)b;  // bをProcess型へキャスト

    return p1->process_time - p2->process_time;  // 処理時間に基づく差分を返す
}

// プロセスリストをリセットする関数
void reset_process_list(Process processList[], Process backupProcessList[], int n){
    // プロセスリスト内の全プロセスについて
    for (int i = 0; i < n; i++){
        // バックアッププロセスリストからプロセスリストへコピー（memcpy関数で構造体全体をコピー）
        memcpy(&processList[i], &backupProcessList[i], sizeof(Process));
    }
}

// FCFSスケジューリング
void FCFS_scheduler(Process processList[], int n){
    // プロセスリストを到着時間でソート
    qsort(processList, n, sizeof(Process), compareByArrivalTime);

    int total_time = 0;             // 全体の時間を初期化
    int total_process_time = 0;     // 全プロセス時間を初期化

    printf("\nFirst-Come-First-Serve\n");  // スケジューリング方法を表示
    printf("start\n");                    // スケジューリング開始を表示
    for (int i = 0; i < n; i++){          // 全プロセスに対して
        // プロセスの到着時間が全体の時間より後の場合、全体の時間を更新
        if (total_time < processList[i].arrival_time){
            total_time = processList[i].arrival_time;
        }
        printf("%2d Task:%s\n", total_time + 1, processList[i].name);  // タスク開始を表示
        total_time += processList[i].process_time;                     // タスクの処理時間を全体の時間に追加
        printf("%2d Task:%s is finish\n", total_time, processList[i].name);  // タスク終了を表示
        processList[i].finish_time = total_time;  // タスクの終了時間を設定
        // 応答時間（タスクの終了時間から到着時間を引いたもの）を計算し、全プロセス時間に追加
        int response_time = processList[i].finish_time - processList[i].arrival_time;
        total_process_time += response_time;
    }
    printf("end\n\n");  // スケジューリング終了を表示

    // スケジューリング結果の表を表示
    printf("Task_Name|Arrival_Time|Finish_Time|Response_time|\n");
    for (int i = 0; i < n; i++) {
        // 各タスクの応答時間を計算
        int response_time = processList[i].finish_time - processList[i].arrival_time;
        printf("        %s|%12d|%12d|%14d|\n", processList[i].name, processList[i].arrival_time, processList[i].finish_time, response_time);
    }

    // 平均応答時間を計算し表示
    printf("\nAverage_Process_Time = %.2f\n", (float)total_process_time / n);
    printf("\n");
}

// SPT（最短処理時間優先）スケジューラ
// 到着時間とプロセス時間に基づいてプロセスを比較する関数
int compare(const void *a, const void *b){
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;

    if (p1->arrival_time < p2->arrival_time){
        return -1;   // 到着時間が小さい方が先に実行されるべきなので -1 を返す
    }else if (p1->arrival_time > p2->arrival_time){
        return 1;    // 到着時間が大きい方が後に実行されるべきなので 1 を返す
    }else{
        // 到着時間が同じ場合、プロセス時間が短い方が先に実行されるべきなので、プロセス時間の差を返す
        return p1->process_time - p2->process_time;
    }
}

void SPT_scheduler(Process processList[], int n){
    // 到着時間順にプロセスリストをソート
    qsort(processList, n, sizeof(Process), compareByArrivalTime);

    // 完了したタスクの情報を保存するためのリストを作成
    Process finishedList[n];
    // 合計時間と合計処理時間を初期化
    int total_time = 0;
    int total_process_time = 0;
    // 元のタスク数を保存
    int n_copy = n;

    // スケジューラの開始を表示
    printf("\nShortest-Process-Time-First\n");
    printf("start\n");

    // 完了したタスクの数をカウント
    int finishCount = 0;

    // まだ処理が未完了のタスクが存在する限りループ
    while (n > 0){
        int idx = -1;
        // すべてのプロセスを調査して、現在の時間に到着し、
        // また最も短い処理時間を持つプロセスを探す
        for (int i = 0; i < n; i++){
            if (processList[i].arrival_time <= total_time){
                if (idx == -1 || processList[i].process_time < processList[idx].process_time)
                    idx = i;
            }
        }
        // そのようなプロセスが見つかった場合
        if (idx != -1){
            // タスクの開始を表示
            printf("%2d Task:%s\n", total_time + 1, processList[idx].name);
            // タスクの処理時間を合計時間に加える
            total_time += processList[idx].process_time;
            // タスクの完了を表示
            printf("%2d Task:%s is finish\n", total_time, processList[idx].name);
            // タスクの完了時間を記録
            processList[idx].finish_time = total_time;
            // 応答時間を計算し、合計処理時間に加える
            int response_time = processList[idx].finish_time - processList[idx].arrival_time;
            total_process_time += response_time;

            // 完了したプロセスの情報をfinishedListに保存
            finishedList[finishCount] = processList[idx];
            finishCount++;

            // 最後のプロセスと完了したプロセスを交換し、プロセス数を減らす
            processList[idx] = processList[n - 1];
            n--;
        }else{
            // タスクがない場合は時間を進める
            total_time++;
        }
    }
    // スケジューラの終了を表示
    printf("end\n\n");

    // 各タスクの名前、到着時間、完了時間、応答時間を表示
    printf("Task_Name|Arrival_Time|Finish_Time|Response_time|\n");
    for (int i = 0; i < n_copy; i++) {
        int response_time = finishedList[i].finish_time - finishedList[i].arrival_time;
        printf("        %s|%12d|%12d|%14d|\n", finishedList[i].name, finishedList[i].arrival_time, finishedList[i].finish_time, response_time);
    }
    // 平均処理時間を表示
    printf("\nAverage_Process_Time = %.2f\n", (float)total_process_time / n_copy);
    printf("\n");
}



// RRスケジューリング
void RR_scheduler(Process processList[], int n, int quantum) {
    int timer = 0;                     // タイマー（全体の時間）を初期化
    int finished_task_number = 0;      // 完了したタスクの数を初期化
    int total_process_time = 0;        // 全プロセス時間を初期化

    printf("\nRound-Robin\n");         // スケジューリング方法を表示
    printf("start\n");                 // スケジューリング開始を表示
    while (finished_task_number != n) { // 全てのタスクが完了するまで
        finished_task_number = 0;      // 完了したタスクの数をリセット
        fetch_new_task(processList, n, timer);  // 新たなタスクをフェッチ
        Process *current_job = dequeue();       // キューから現在のタスクを取り出す
        if (current_job == NULL) {              // キューが空である場合（まだ到着するタスクがない場合）
            ++timer;                            // タイマーを進める
            for (int i = 0; i < n; ++i) {
                if(processList[i].status == 2)
                    ++finished_task_number;     // 完了したタスクの数をカウント
            }
            if (finished_task_number != n) {    // 全てのタスクが完了していない場合
                if (timer)
                    printf("%2d skip\n", timer); // スキップすることを表示
                continue;                       // 次のループに進む
            } else {
                break;                          // 全てのタスクが完了したらループを抜ける
            }
        }

        int counter = timer + quantum;          // タスクが終了する予定の時間を計算
        if (current_job->remaining_time <= quantum)  // タスクの残り時間がタイムクォンタム以下の場合
            counter = timer + current_job->remaining_time;  // タスクが終了する予定の時間を再計算
        while (timer < counter) {               // タスクが終了する予定の時間になるまで
            ++timer;                            // タイマーを進める
            --current_job->remaining_time;     // タスクの残り時間を減らす
            if (current_job->remaining_time != 0)
                printf("%2d Task:%s\n", timer, current_job->name);  // タスク進行中を表示
            fetch_new_task(processList, n, timer);  // 新たなタスクをフェッチ
        }
        if (current_job->remaining_time == 0) {  // タスクが終了した場合
            current_job->status = 2;             // タスクの状態を更新
            printf("%2d Task:%s is finish\n", timer, current_job->name);  // タスク終了を表示
            current_job->finish_time = timer;    // タスクの終了時間を設定
            int response_time = current_job->finish_time - current_job->arrival_time;  // 応答時間を計算
            total_process_time += response_time;  // 全プロセス時間を更新
        } else {
            enqueue(current_job);  // タスクが終了していない場合、再度キューに追加
        }
        for (int i = 0; i < n; ++i) {
            if (processList[i].status == 2)
                ++finished_task_number;  // 完了したタスクの数をカウント
        }
    }
    printf("end\n\n");   // スケジューリング終了を表示

    // スケジューリング結果の表を表示
    printf("Task_Name|Arrival_Time|Finish_Time|Response_time|\n");
    for (int i = 0; i < n; i++) {
        int response_time = processList[i].finish_time - processList[i].arrival_time;  // 応答時間を計算
        printf("        %s|%12d|%12d|%14d|\n", processList[i].name, processList[i].arrival_time, processList[i].finish_time, response_time);
    }
    // 平均プロセス時間を計算し表示
    printf("\nAverage_Process_Time = %.2f\n", (float)total_process_time / n);
}


int main(void){
    memcpy(backupProcessList, processList, sizeof(processList));

    FCFS_scheduler(processList, PROCESS_NUM);
    reset_process_list(processList, backupProcessList, PROCESS_NUM);

    SPT_scheduler(processList, PROCESS_NUM);
    reset_process_list(processList, backupProcessList, PROCESS_NUM);

    RR_scheduler(processList, PROCESS_NUM, TIME_QUANTUM);
    return 0;
}
