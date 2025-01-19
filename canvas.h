/**
 * @brief Drone_demo project
 * @author B.Piranda ---STUDENTS-ZAHRAHMAN Bilal & ABIONA Boluwatife
 * @date dec. 2024
 **/
#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <drone.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVector>
#include <QColor>
#include <QMap>
#include <QSet>
#include <QString>
#include "vector2d.h"
class QPainter;
class Canvas : public QWidget {

    Q_OBJECT
public:
    const int droneIconSize=64; ///< size of the drone picture in the vanvas
    const double droneCollisionDistance=droneIconSize*1.5; ///< distance to detect collision with other drone
    /**
     * @brief Canvas constructor
     * @param parent
     */
    explicit Canvas(QWidget *parent = nullptr);
    /**
     * @brief setMap set the list of drones (identified by their name) to the canvas
     * @param map the map of couple "name of the drone"/"drone pointer"
     */
    inline void setMap(QMap<QString,Drone*> *map) { mapDrones=map; }
    /**
     * @brief paintEvent
     */
    void paintEvent(QPaintEvent*) override;
    /**
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief loadJsonData loads server and drone information from a JSON file
     * @param jsonFilePath the path to the JSON file
     */
     void loadJsonData(const QString &jsonFilePath = ""); // Function to load JSON data

    /**
      * @brief initializeServerConnections
      */
     void initializeServerConnections();  // Method to initialize server connectivity

     /**
    /**      * @brief getNextServerComputes the next server for a drone based on current and target servers.
    /**      * @param current Current server name.
    /**      * @param target the Target server name
    /**      * @return next server name in oath
    /**      */

     QString getNextServer(const QString &current, const QString &target);  // Logic for moving drones
     /**
      * @brief parseServerConnections from a json object
      * @param jsonObject
      */
     void parseServerConnections(const QJsonObject &jsonObject);  // Declare function to parse connections
     /**
      * @brief computeVoronoiPolygons for servers
      */
     void computeVoronoiPolygons();
     /**
      * @brief check if polygon sare an edge
      * @return return tre if they share an edge otherwise false
      */
     bool polygonsShareEdge(const QPolygonF &poly1, const QPolygonF &poly2);

     /**
     * @brief Finds a path between two servers using connectivity data.
     * @param start Starting server name.
     * @param goalis to Target server name.
     * @return List of server names representing the path.
     */
     QStringList findPathBasedOnConnections(const QString &start, const QString &goal);
     /**
     * @brief Updates the target server for a given drone.
     */
     void updateDroneTarget(Drone *drone);
     /**
     * @brief Finds the current server for a drone.
     * @return Name of the current server.
     */
     QString getCurrentServerForDrone(Drone *drone);  // Helper to find the current server of a drone
     /**
     * @brief Finds the target server for a drone.
     * @return Name of the target server.
     */
     QString getTargetServerForDrone(Drone *drone);   // Helper to get target server for a drone

     /**
     * @brief Gets the name of the target server.
     */
     QString getTargetServerName() const;

    QPolygonF getServerPolygon(const Vector2D &serverPos);


  public:
     /**
     * @brief Represents a server with name, position, color, and polygon.
     */
     struct Server {
        QString name;
        Vector2D position;
        QColor color;
        QPolygonF polygon;


    };
    QVector<Server> servers;// List of servers
signals:

private:
    /** Draw the Voronoi diagram on the canvas */
    void drawVoronoiDiagram(QPainter& painter);
    /**
     * @brief drawServerConnections
     * @param painter
     */
    void drawServerConnections(QPainter& painter);
    /**
     * @brief getServerByName
     * @param name Server name
     * @return Pointer to the server or nullptr if not found.
     */
    Server* getServerByName(const QString &name);
    /**
     * @brief drawServers Draws the server polygons
     * @param painter QPainter object used to  draw on the canvas
     */
    void drawServers(QPainter &painter);

    QVector<Drone*> drones;//list of drones
    //QVector<Server> servers;  // List of servers
    QMap<QString,Drone*> *mapDrones=nullptr; //pointer on the map of the drones
    QMap<QString, QSet<QString>> serverConnections;// Adjacency list for server connections
    QImage droneImg; ///< picture representing the drone in the canvas

    /**
     * @brief euclideanDistance
     * @param a
     * @param b
     * @return Distance between the points.
     */
    static double euclideanDistance(const Vector2D &a, const Vector2D &b);  // Static helper for distance
    /**
     * @brief parsePosition
     * @param position String representation of position.
     * @return  representing the parsed position.
     */
    QPoint parsePosition(const QString &position);  // Helper to parse position strings
    /**
     * @brief selectedDrone Pointer to the currentselected drone.
     */
    Drone* selectedDrone = nullptr;
    /**
     * @brief activeDrone pointer to the curren active drone.
     */
    Drone* activeDrone = nullptr;  // Track the currently selected drone





};

#endif // CANVAS_H
