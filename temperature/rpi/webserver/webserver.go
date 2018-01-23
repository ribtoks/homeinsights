package main

import (
  "database/sql"
  _ "github.com/mattn/go-sqlite3"
  "encoding/json"
  "net/http"
  "bytes"
  "time"
  "flag"
)

var (
  dbPathFlag = flag.String("db-path", "./temp.db", "Path to the database with temperature notes")
)

type TempData struct {
  Time string `json:"time"`
  SensorID int `json:"sensorID"`
  Temperature float32 `json:"temperature"`
}

type TempHandler struct {
  db *sql.DB
  selectStmt *sql.Stmt
}

func (th *TempHandler) initDatabase() error {
  var err error
  th.db, err = sql.Open("sqlite3", *dbPathFlag)
  if err != nil {
    return err
  }
  
  th.selectStmt, err = th.db.Prepare("SELECT * FROM Temps")
  if err != nil {
    return err
  }

  return nil
}

func (th *TempHandler) finalizeDatabase() error {
  th.db.Close()
  return nil
}

func (th *TempHandler) ServeHTTP(rw http.ResponseWriter, request *http.Request) {
  var timestamp int64
  var sensorID int
  var temperature float32

  rows, err := th.selectStmt.Query()
  if err != nil { return }

  var tempsArr []TempData
  
  for rows.Next() {
    err = rows.Scan(&timestamp, &sensorID, &temperature)
    if err != nil { continue }

    t := time.Unix(timestamp, 0)        
    tempsArr = append(tempsArr, TempData{t.String(), sensorID, temperature})
  }

  temps, _ := json.Marshal(tempsArr)
  byteArray := bytes.NewBuffer(temps).Bytes()

  rw.Write(byteArray)
  
  rows.Close()
}

func main() {
  flag.Parse()

  handler := &TempHandler{}
  err := handler.initDatabase()
  if err != nil {
    return
  }

  http.Handle("/", http.FileServer(http.Dir("./www")))
  http.Handle("/temps", handler)

  err = http.ListenAndServe(":8080", nil)

  handler.finalizeDatabase();
}
