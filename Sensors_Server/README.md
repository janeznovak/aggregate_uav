# Getting Started

## Description
HTTP Server using REST protocol to serve PoC project. It uses JPA to persist in a in-memory database information, like data coming from sensors and new goals.

## Parameters (env variable)
- SQL_INIT_DATA: custom initial data of database, eg. location of items, etc... (DEFAULT: data-library.sql)
- APP_ROBOT_EXAMPLE: format of node name (DEFAULT: tb3_#ROBOT)
- APP_ROBOT_COUNT: number of robots configured (DEFAULT: 10)
- APP_WEARABLE_EXAMPLE: format of wearable name (DEFAULT: wb_#WEARABLE)
- APP_WEARABLE_COUNT: number of wearables configured (DEFAULT: 10)
- APP_STORAGE_PATH: path of the Storage used (DEFAULT: ../Storage)
- APP_SCRIPTS_PATH: path of the bash scripts used by java server (DEFAULT: ../PoC)
- APP_GOAL_PATH_TO_WRITE: path of the goals used to acquire files (DEFAULT: APP_STORAGE_PATH/from_user/goals/)
- APP_ACTION_PATH_TO_READ: path of the actions used to acquire files (DEFAULT: APP_STORAGE_PATH/from_ap/to_web/actions/#ROBOT/)
- APP_FEEDBACK_PATH_TO_READ: path of the feedback used to acquire files (DEFAULT: APP_STORAGE_PATH/from_robot/#ROBOT/to_web/feedback/)
- APP_WEARABLE_FEEDBACK_PATH_TO_READ: path of the wearable feedbacks used to acquire files (DEFAULT: APP_STORAGE_PATH/from_wearable/#WEARABLE/to_web/feedback/)
- APP_WEARABLE_SOS_PATH_TO_READ: path of the wearable sos used to acquire files (DEFAULT: APP_STORAGE_PATH/from_wearable/#WEARABLE/to_web/sos/)

## Run without parameters (using YAML configuration)
```bash
java -jar binary/java-sensors-server-0.0.1-SNAPSHOT.jar
```
or

```bash
mvn spring-boot:run
```

## Run with parameters
```bash
java -DAPP_STORAGE_PATH=$PATH_STORAGE -DAPP_ROBOT_EXAMPLE=${PREFIX_ROBOT}_#ROBOT -jar binary/java-sensors-server-0.0.1-SNAPSHOT.jar
```
or

```bash
mvn spring-boot:run -Dspring-boot.run.arguments=--app.storage.path=$PATH_STORAGE
```

### Links provided by the application
http://localhost:8080/swagger-ui/index.html   
http://localhost:8080/h2-console

## Example of run
ITEM: `ITEM_1, ITEM_2, ITEM_3, ITEM_4, ITEM_5, ITEM_6, ITEM7`

curl -X POST "http://localhost:8080/goals" -H "accept: application/json" -H "Content-Type: application/json" -d "{ \"action\": \"GOAL\", \"goalCode\": \"GOAL_1\", \"referenceCode\": \"ITEM_1\"}"

# Docker
```bash
# update build
docker build -t nodes/sensors_server .

# run docker
docker run -p 8080:8080 -p 7400:7400 nodes/sensors_server
```