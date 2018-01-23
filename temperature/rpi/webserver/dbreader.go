package main

import (
  "database/sql"
  _ "github.com/mattn/go-sqlite3"
  "encoding/json"
  "net/http"
  "bytes"
  "time"
)

type TempData struct {
  Time string `json:"time"`
  SensorID int `json:"sensorID"`
  Temperature float32 `json:"temperature"`
}

func main() {

  http.HandleFunc("/temps", handler)

  err := http.ListenAndServe(":6688", nil)
  checkErr(err)  
  
}

func handler(rw http.ResponseWriter, request *http.Request) {

  db, err := sql.Open("sqlite3", "./temp.db")
  checkErr(err)

  statement, err := db.Prepare("SELECT * FROM Temps")
  checkErr(err)
  
  var timestamp int64
  var sensorID int
  var temperature float32

  rows, err := statement.Query()
  checkErr(err)

  var tempsArr []TempData
  
  for rows.Next() {
    err = rows.Scan(&timestamp, &sensorID, &temperature)
    checkErr(err)

    t := time.Unix(timestamp, 0)
        
    tempsArr = append(tempsArr, TempData{t.String(), sensorID, temperature})
  }

  temps, _ := json.Marshal(tempsArr)
  byteArray := bytes.NewBuffer(temps).Bytes()

  rw.Write(byteArray)
  
  rows.Close()

  db.Close()
}

func checkErr(err error) {
  if err != nil {
    panic(err)
  }
}
