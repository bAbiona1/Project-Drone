/**
 * @brief Drone_demo project
 * @author B.Piranda ---STUDENTS-ZAHRAHMAN Bilal & ABIONA Boluwatife
 * @date dec. 2024
 **/
#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>

class Vector2D {
public:
    float x,y; ///< coordinates of the vector
    Vector2D(float p_x,float p_y):x(p_x),y(p_y) {};
    Vector2D() { x=y=0.0; }
    Vector2D(Vector2D *p):x(p->x),y(p->y) {};

    /**
     * @brief set new coordinates to the vector
     * @param p_x: x component
     * @param p_y: y component
     */
    void set(float p_x,float p_y) { x=p_x; y=p_y; }
    /**
     * @brief length: return the length (or the norm) of the vector
     * @return the length
     */
    double length() const {
        return sqrt(x*x+y*y);
    }
    /**
     * @brief normalize: change the current vector to a vector with the same direction but a norme equal to 1
     */
    void normalize() {
        float l=float(length());
        x/=l;
        y/=l;
    }
    /**
     * @brief orthoNormed: return a new vector, orthogonal and normed
     * @return the orthonormed vector
     */
    Vector2D orthoNormed() const {
        float l=float(length());
        return Vector2D(y/l,-x/l);
    }
    /**
     * @brief operator []: a way to get the component of the vector
     * @param i: equal to 0 or 1
     * @return x if i is equal to 0 and y else
     */
    float operator[](const int i) const {
        return (i==0)?x:y;
    };
    /**
     * @brief operator +=: multiply the components of the current vector with the components of the vector v
     * @param v: the multiplier
     */
    void operator+=(const Vector2D& v) {
        x+=v.x;
        y+=v.y;
    }

    Vector2D operator*(double scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }


    Vector2D operator/(double scalar) const {
        return Vector2D(x / scalar, y / scalar);
    }



    friend double operator *(const Vector2D&,const Vector2D&);
    friend const Vector2D operator *(double,const Vector2D&);
    friend const Vector2D operator +(const Vector2D&,const Vector2D&);
    friend const Vector2D operator -(const Vector2D&,const Vector2D&);
    friend const Vector2D operator -(const Vector2D&);
    friend double operator ^(const Vector2D&,const Vector2D&);
    friend bool operator ==(const Vector2D&,const Vector2D&);
    friend bool operator !=(const Vector2D&,const Vector2D&);
};

const Vector2D operator *(double,const Vector2D&);
double operator *(const Vector2D&,const Vector2D&);
const Vector2D operator +(const Vector2D&,const Vector2D&);
const Vector2D operator -(const Vector2D&,const Vector2D&);
const Vector2D operator -(const Vector2D&);
double operator ^(const Vector2D&,const Vector2D&);
bool operator==(const Vector2D&,const Vector2D&);
bool operator !=(const Vector2D&,const Vector2D&);


#endif // VECTOR2D_H
