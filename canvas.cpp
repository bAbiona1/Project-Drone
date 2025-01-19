#include "canvas.h"
#include <QPainter>
#include "drone.h"
#include <cmath>
#include <limits>
#include <QDebug>
#include <fstream>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QQueue>


/**
 * @brief Calculates the Euclidean distance between two 2D points.
 * @param a The first point as a Vector2D object.
 * @param b The second point as a Vector2D object.
 * @return The Euclidean distance between the two points.
 */
 double Canvas::euclideanDistance(const Vector2D& a, const Vector2D& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}



/**
 * @brief Constructor for the Canvas class.
 * Initializes the canvas and loads the drone image.
 * @param parent The parent QWidget object.
 */
Canvas::Canvas(QWidget *parent)
    : QWidget{parent} {
    droneImg.load("../../media/drone.png");
    setMouseTracking(true);
}


/**
 * @brief Loads data from a JSON file and updates the drone and server information.
 * updates the positions and goal locations for the drones.
 * It also assigns servers to drones.
 * @param jsonFilePath The path to the JSON file to be loaded.
 */
void Canvas::loadJsonData(const QString &jsonFilePath) {
    QString filePath = jsonFilePath;

    // Open file dialog if no path is provided
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(
            this,
            tr("Open JSON File"),
            "",
            tr("JSON Files (*.json);;All Files (*)")
            );

        if (filePath.isEmpty()) {
            QMessageBox::warning(this, tr("File Error"), tr("No file selected."));
            return;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("File Error"), tr("Couldn't open the selected JSON file."));
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON data
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, tr("JSON Error"), tr("Invalid JSON file format."));
        return;
    }

    QJsonObject rootObj = doc.object();

    // Parse servers
    if (rootObj.contains("servers") && rootObj["servers"].isArray()) {
        QJsonArray serversArray = rootObj["servers"].toArray();
        servers.clear();

        for (const QJsonValue &serverValue : serversArray) {
            QJsonObject serverObj = serverValue.toObject();
            QString name = serverObj["name"].toString();
            QStringList positionStr = serverObj["position"].toString().split(",");
            QColor color(serverObj["color"].toString());

            if (positionStr.size() == 2) {
                int x = positionStr[0].toInt();
                int y = positionStr[1].toInt();

                Server server;
                server.name = name;
                server.position = Vector2D(x, y);
                server.color = color;

                servers.append(server);

                qDebug() << "Server loaded:" << name << "Position:" << x << "," << y << "Color:" << color;
            }
        }
    } else {
        QMessageBox::warning(this, tr("JSON Error"), tr("No 'servers' array found in the JSON file."));
    }

    // Parse drones and position them
    if (rootObj.contains("drones") && rootObj["drones"].isArray()) {
        QJsonArray dronesArray = rootObj["drones"].toArray();
        drones.clear(); // Clear existing drones

        for (const QJsonValue &droneValue : dronesArray) {
            QJsonObject droneObj = droneValue.toObject();
            QString name = droneObj["name"].toString();
            QString targetServerName = droneObj["server"].toString();

            QStringList positionStr = droneObj["position"].toString().split(",");
            if (positionStr.size() == 2) {
                int x = positionStr[0].toInt();
                int y = positionStr[1].toInt();

                Drone *drone = new Drone(name);
                //Drone *drone = new Drone(name, this);



                drone->setInitialPosition(Vector2D(x, y));

                // Find the specified server for the drone
                Server *targetServer = getServerByName(targetServerName);
                if (targetServer) {
                    drone->setGoalPosition(targetServer->position);
                     drone->setTargetServerName(targetServerName);
                    qDebug() << "Drone" << name << "assigned to server" << targetServerName;
                } else {
                    qDebug() << "Server" << targetServerName << "not found for drone" << name;
                }

                drones.append(drone); // Add the drone to the list
            }
        }
    }

    // Update the map of drones for MainWindow
    if (mapDrones) {
        mapDrones->clear();
        for (auto &drone : drones) {
            mapDrones->insert(drone->getName(), drone); // Add each drone to the map
        }
    }
    parseServerConnections(rootObj);
    update(); // repaint to show the updated positions of drones and Voronoi regions
}

/**
 * @brief Canvas::paintEvent
 */

/*paintevent
void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QBrush whiteBrush(Qt::SolidPattern);
    QPen penCol(Qt::DashDotDotLine);
    penCol.setColor(Qt::lightGray);
    penCol.setWidth(3);
    whiteBrush.setColor(Qt::white);
    painter.fillRect(0,0,width(),height(),whiteBrush);


    //Draw the Voronoi diagram

    drawVoronoiDiagram(painter);
    //draw server connections
    drawServerConnections(painter);

    if (mapDrones) {
        Vector2D p;
        QRect rect(-droneIconSize/2,-droneIconSize/2,droneIconSize,droneIconSize);
        QRect rectCol(-droneCollisionDistance/2,-droneCollisionDistance/2,droneCollisionDistance,droneCollisionDistance);

        for (auto &drone:*mapDrones) {
            updateDroneTarget(drone);
            painter.save();
            // place and orient the drone
            painter.translate(drone->getPosition().x,drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rect,droneImg);
            // light leds if flying
            if (drone->getStatus()!=Drone::landed) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawEllipse((-185.0/511.0)*droneIconSize,(-185.0/511.0)*droneIconSize,(65.0/511.0)*droneIconSize,(65.0/511.0)*droneIconSize);
                painter.drawEllipse((115.0/511.0)*droneIconSize,(-185.0/511.0)*droneIconSize,(65.0/511.0)*droneIconSize,(65.0/511.0)*droneIconSize);
                painter.setBrush(Qt::green);
                painter.drawEllipse((-185.0/511.0)*droneIconSize,(115.0/511.0)*droneIconSize,(70.0/511.0)*droneIconSize,(70.0/511.0)*droneIconSize);
                painter.drawEllipse((115.0/511.0)*droneIconSize,(115.0/511.0)*droneIconSize,(70.0/511.0)*droneIconSize,(70.0/511.0)*droneIconSize);
            }
            // draw collision detector
            if (drone->hasCollision()) {
                painter.setPen(penCol);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(rectCol);
            }
            painter.restore();
        }
    }
}
/*



//----
/**
 * @brief Handles the mouse press event to select and move drones.
 * @param event The mouse event triggered by a click.
 */

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // Set up the background and basic styles
    QBrush whiteBrush(Qt::SolidPattern);
    QPen penCol(Qt::DashDotDotLine);
    penCol.setColor(Qt::lightGray);
    penCol.setWidth(3);
    whiteBrush.setColor(Qt::white);
    painter.fillRect(0, 0, width(), height(), whiteBrush);

    // Draw the Voronoi diagram
    drawVoronoiDiagram(painter);

    // Draw server connections
    drawServerConnections(painter);

    // Draw the servers as clickable polygons
    drawServers(painter);  // function to draw the server polygons

    // Draw drones (if any)
    if (mapDrones) {
        Vector2D p;
        QRect rect(-droneIconSize / 2, -droneIconSize / 2, droneIconSize, droneIconSize);
        QRect rectCol(-droneCollisionDistance / 2, -droneCollisionDistance / 2, droneCollisionDistance, droneCollisionDistance);

        for (auto &drone : *mapDrones) {
            updateDroneTarget(drone);
            painter.save();
            // Place and orient the drone
            painter.translate(drone->getPosition().x, drone->getPosition().y);
            painter.rotate(drone->getAzimut());
            painter.drawImage(rect, droneImg);

            // Light LEDs if flying
            if (drone->getStatus() != Drone::landed) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::red);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (-185.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize, (65.0 / 511.0) * droneIconSize);
                painter.setBrush(Qt::green);
                painter.drawEllipse((-185.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
                painter.drawEllipse((115.0 / 511.0) * droneIconSize, (115.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize, (70.0 / 511.0) * droneIconSize);
            }

            // Draw collision detector
            if (drone->hasCollision()) {
                painter.setPen(penCol);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(rectCol);
            }
            painter.restore();
        }
    }
}

/**
 * @brief Canvas::drawServers function to
 * draw servers as clickable polygons
 * @param painter
 */
void Canvas::drawServers(QPainter &painter) {
    QPen serverPen(Qt::black);  //  color for the server polygons
    painter.setPen(serverPen);
    painter.setBrush(Qt::NoBrush);

    for (const Server &server : servers) {

        QPointF serverPos(server.position.x, server.position.y);
        qreal radius = 30;

        // Draw server as a circle
        painter.drawEllipse(serverPos, radius, radius);
    }
}


/* mousePressEvent
void Canvas::mousePressEvent(QMouseEvent *event) {
    // search for a drone that is landed
    auto it = mapDrones->begin();
    while (it!=mapDrones->end() && (*it)->getStatus()!=Drone::landed) {
        it++;
    }
    // if found, ask for a motion to the mouse position
    if (it!=mapDrones->end()) {
        (*it)->setGoalPosition(Vector2D(event->pos().x(),event->pos().y()));
        (*it)->start();
    }
    repaint();
}
*/
//----
///*

/**
 * @brief Canvas::mousePressEvent Handles mouse press events for activating drones and setting their target servers.
 * this function checks for the mouse press event and does two action which is
 * 1-if a drone is clicked,it activate drone and sets it as the the active drone,
 * 2- if a server is clicked and a drone is active it then set the server which is clicked as the target of the the active drone
 * and updates the drones movement toward the server that is being targetted .
 * @param event
 */
void Canvas::mousePressEvent(QMouseEvent *event) {
    QPointF clickPos = event->pos();

    //Check if a drone is clicked
    for (Drone* drone : *mapDrones) {
        QRect droneBounds(drone->getPosition().x - 20, drone->getPosition().y - 20, 40, 40);
        if (droneBounds.contains(clickPos.toPoint())) {
            if (activeDrone) {
                qDebug() << "Deactivating drone:" << activeDrone->getName();
            }
            activeDrone = drone;
            qDebug() << "Drone activated:" << activeDrone->getName();
            return;
        }
    }

    //  If a server is clicked and a drone is active
    if (activeDrone) {
        for (const Server &server : servers) {
            QPointF serverPos(server.position.x, server.position.y);
            qreal radius = 30;

            // Check if the click is within the server's circle
            if ((clickPos - serverPos).manhattanLength() <= radius) {
                activeDrone->setTargetServerName(server.name);  // Set the target server for the active drone
                updateDroneTarget(activeDrone);  // Move the drone
                activeDrone->start();
                qDebug() << "Drone" << activeDrone->getName() << "moving to server:" << server.name;
                activeDrone = nullptr;  // Reset active drone
                return;
        }
    }
}

}
//*/

/**
 * @brief Canvas::drawVoronoiDiagram Draws the Voronoi diagram based on  drone positions.
 * @param painter The QPainter object used to draw the Voronoi diagram.
 */
void Canvas::drawVoronoiDiagram(QPainter &painter) {
    if (servers.isEmpty()) return;

    int resolution = 1;
    // Loop through every point in the grid
    for (int x = 0; x < width(); x += resolution) {
        for (int y = 0; y < height(); y += resolution) {
            Vector2D currentPoint(x, y);
            double minDistance = std::numeric_limits<double>::max();
            QColor closestColor;

            // Find the closest server
            for (const auto &server : servers) {
                double distance = euclideanDistance(currentPoint, server.position);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestColor = server.color;
                }
            }

            //  represent the region
            painter.fillRect(x, y, resolution, resolution, closestColor);
        }
    }

    // Draw server positions
    for (const auto &server : servers) {
        painter.setBrush(Qt::black);
        painter.setPen(Qt::black);
        painter.drawEllipse(QPointF(server.position.x, server.position.y), 10, 10); // Circle at server position

        // Draw the server name near its position
        painter.setPen(Qt::white);
        painter.drawText(server.position.x + 15, server.position.y - 15, server.name);
    }
}



/**
 * @brief Canvas::getServerByName this function searches for a server by its name from the list of servers.
 * @param name The name of the server to retrieve.
 * @return A pointer to the Server object if found.
 */

Canvas::Server* Canvas::getServerByName(const QString &name) {
    for (auto &server : servers) {
        if (server.name == name) {
            return &server; // Return  matching server
        }
    }
    return nullptr; // Return nullptr if no match is found
}



/**
 * @brief Canvas::parseServerConnections this function parseServer connectionfrom the provided JSON object and
 * processes the server connections specified in the JSON data,
 * @param jsonObject The root JSON object that contains the server connection information
 */
void Canvas::parseServerConnections(const QJsonObject &jsonObject) {
    serverConnections.clear();
    const auto serverList = jsonObject["servers"].toArray();
    for (int i = 0; i < serverList.size(); ++i) {
        QJsonObject serverA = serverList[i].toObject();
        QString serverAName = serverA["name"].toString();
        QPoint serverAPosition = parsePosition(serverA["position"].toString());

        for (int j = i + 1; j < serverList.size(); ++j) {
            QJsonObject serverB = serverList[j].toObject();
            QString serverBName = serverB["name"].toString();
            QPoint serverBPosition = parsePosition(serverB["position"].toString());

            // Calculate the distance between two servers
            double distance = std::hypot(serverAPosition.x() - serverBPosition.x(),
                                         serverAPosition.y() - serverBPosition.y());


            if (distance < 500) {
                serverConnections[serverAName].insert(serverBName);
                serverConnections[serverBName].insert(serverAName); // Bidirectional connection
            }
        }
    }
}



/**
 * @brief Canvas::parsePosition
 * @param position
 * @return
 */

QPoint Canvas::parsePosition(const QString &position) {
    QStringList coords = position.split(",");
    if (coords.size() == 2) {
        return QPoint(coords[0].toInt(), coords[1].toInt());
    }
    return QPoint();
}



/**
 * @brief Canvas::drawServerConnections draws the server connections on the canvas by looping through each server and the set of connected server then draws conecting line between
 * each pairs of cnnected servers
 * @param painter the QPainter objects is then  used to draw the server connections.
 */

void Canvas::drawServerConnections(QPainter &painter) {
    for (auto it = serverConnections.begin(); it != serverConnections.end(); ++it) {
        QString serverNameA = it.key();
        Server *serverA = getServerByName(serverNameA);

        if (serverA) {
            for (const QString &serverNameB : it.value()) {
                if (serverNameA != serverNameB) {
                    Server *serverB = getServerByName(serverNameB);
                    if (serverB) {
                        painter.setPen(QPen(Qt::white, 2));
                        painter.drawLine(serverA->position.x, serverA->position.y,
                                         serverB->position.x, serverB->position.y);
                    }
                }
            }
        }
    }
}



/**
 * @brief Canvas::findPathBasedOnConnections this function Finds the shortest path between two servers based on their connections.
 * used the BFS -breadth first seach to fine shortest path from the start server and goal server basd on the connection btw servers
 * @param start the starting server name
 * @param goal the goal server name
 * @return  A list of server names representing the path from start to goal, or an empty list if no path is found.
 */
QStringList Canvas::findPathBasedOnConnections(const QString &start, const QString &goal) {
    if (start == goal) {
        return {start};
    }
    if (!serverConnections.contains(start) || !serverConnections.contains(goal)) {
        return {};
    }

    QQueue<QString> queue;
    QMap<QString, QString> predecessors;
    QSet<QString> visited;

    queue.enqueue(start);
    visited.insert(start);

    while (!queue.isEmpty()) {
        QString current = queue.dequeue();
        for (const QString &neighbor : serverConnections[current]) {
            if (!visited.contains(neighbor)) {
                predecessors[neighbor] = current;
                if (neighbor == goal) {
                    QStringList path;
                    QString step = neighbor;
                    while (!step.isEmpty()) {
                        path.prepend(step);
                        step = predecessors.value(step, "");
                    }
                    path.prepend(start);
                    return path;
                }
                visited.insert(neighbor);
                queue.enqueue(neighbor);
            }
        }
    }

    return {};
}


/**
 * @brief Canvas::updateDroneTarget this function updates the target positioning of a drone based on the current and target servers.
 * @param drone The drone object whose target position is to be updated.
 */

void Canvas::updateDroneTarget(Drone *drone) {
    QString currentServer = getCurrentServerForDrone(drone);
    QString targetServer = getTargetServerForDrone(drone);

    if (currentServer.isEmpty() || targetServer.isEmpty()) {
        return;  // No valid movement if drone isn’t on a server
    }

    // Get the path from current to target
    QStringList path = findPathBasedOnConnections(currentServer, targetServer);

    if (path.size() > 1) {
        for (int i = 1; i < path.size(); i++) {
            Server *nextServerObj = getServerByName(path[i]);
            if (nextServerObj) {
                drone->setGoalPosition(nextServerObj->position);  // Move to each intermediate step
                 drone->setTargetServerName(nextServerObj->name);
            }
        }
    }
}


/**
 * @brief Canvas::getNextServer this function is to fnd next server in the path between the current server and the target server and
 * returns the next server along the path from the current server to the target server,
 * based on the connections between servers.
 * @param current current server name
 * @param target target server name
 * @return The name of the next server in the path, or it will return an  empty string if there is no valid next server  found.
 */
QString Canvas::getNextServer(const QString &current, const QString &target) {
    QStringList path = findPathBasedOnConnections(current, target);
    qDebug() << "Path from" << current << "to" << target << ":" << path;

    if (path.size() > 1) {
        qDebug() << "Next step is:" << path[1];
        return path[1];  // Move to the next step
    }
    qDebug() << "No valid path found!";
    return {};
}

/**
 * @brief Canvas::getCurrentServerForDrone this function checks the position of the drone for any given drone and finds the server that contains the position
 * @param drone The drone object for which to determine the current server.
 * @return The name of the current server the drone is located in, or return an empty string if it is not  found.
 */
QString Canvas::getCurrentServerForDrone(Drone *drone) {
    Vector2D position = drone->getPosition();

    // check if the drone is inside a server polygon
    for (const auto &server : servers) {
        if (server.polygon.containsPoint(QPointF(position.x, position.y), Qt::WindingFill)) {
            return server.name;
        }
    }

    // If not inside any polygon, find the nearest server
    double minDistance = std::numeric_limits<double>::max();
    QString closestServer;

    for (const auto &server : servers) {
        double distance = euclideanDistance(position, server.position);
        if (distance < minDistance) {
            minDistance = distance;
            closestServer = server.name;
        }
    }

    return closestServer;
}

/**
 * @brief Canvas::getTargetServerForDrone ths function gets the targer server for a drone
 * @param drone the drone object for which to retrieve the target server name.
 * @return The name of the target server for the drone.
 */
QString Canvas::getTargetServerForDrone(Drone *drone) {
    return drone->getTargetServerName();
}

/**
 * @brief Canvas::getServerPolygon
 * @param serverPos
 * @return
 */
QPolygonF Canvas::getServerPolygon(const Vector2D &serverPos) {
    for (const auto &server : servers) {
        if ((server.position - serverPos).length() < 10.0) {  // Find the matching server
            return server.polygon;  // Return its Voronoi polygon
        }
    }
    return QPolygonF();  // Return  empty polygon  if not found
}




