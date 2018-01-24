package main

import (
  "database/sql"
  _ "github.com/mattn/go-sqlite3"
  "encoding/json"
  "net/http"
  "bytes"
//  "time"
  "flag"
  "log"
  "os"
  "fmt"
)

var (
  dbPathFlag = flag.String("db-path", "./temp.db", "Path to the database with temperature notes")
)

type TempData struct {
  Time int64 `json:"time"`
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

  log.Println("Database initialized")
  
  return nil
}

func (th *TempHandler) finalizeDatabase() error {
  th.db.Close()
  
  log.Println("Database closed")
  
  return nil
}

func (th *TempHandler) ServeHTTP(rw http.ResponseWriter, request *http.Request) {
  var timestamp int64
  var sensorID int
  var temperature float32
  
  log.Println("Recieved request")
  
  rows, err := th.selectStmt.Query()
  if err != nil { return }

  var tempsArr []TempData
  
  for rows.Next() {
    err = rows.Scan(&timestamp, &sensorID, &temperature)
    if err != nil { continue }

    //t := time.Unix(timestamp, 0)        
    tempsArr = append(tempsArr, TempData{timestamp, sensorID, temperature})
  }

  temps, _ := json.Marshal(tempsArr)
  byteArray := bytes.NewBuffer(temps).Bytes()

  rw.Write(byteArray)
  
  rows.Close()
}

func main() {
  flag.Parse()

  logfile, err := setupLogging()
  if err != nil {
    defer logfile.Close()
  }
  
  handler := &TempHandler{}
  err = handler.initDatabase()
  if err != nil {
    return
  }

  http.Handle("/", http.FileServer(http.Dir("./www")))
  http.Handle("/temps", handler)

  err = http.ListenAndServe(":8080", nil)

  handler.finalizeDatabase();
}

func setupLogging() (f *os.File, err error) {
  f, err = os.OpenFile("./logfile.log", os.O_RDWR | os.O_CREATE | os.O_APPEND, 0666)
  if err != nil {
    fmt.Println("error opening file: %v", "./logfile.log")
    return nil, err
  }

  log.SetOutput(f)  

  log.Println("------------------------------")
  log.Println("log started")

  return f, err
}
