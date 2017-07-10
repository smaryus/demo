#include <stdio.h>

class Feature
{
public:
    enum FeatureType {eUnknown, eCircle, eTriangle, eSquare};

    Feature() : points(nullptr), type(eUnknown) { }

    ~Feature()
    {
        reset();
    }

    bool isValid() const
    {
        return (type != eUnknown) && (points != nullptr);
    }

    bool read(FILE* file)
    {
        reset();

        if( !file )
        {
            return false;
        }

        if (fread(&type, sizeof(FeatureType), 1, file) != 1)
        {
            return false;
        }

        short n = 0;
        switch (type)
        {
            case eCircle: n = 3; break;
            case eTriangle: n = 6; break;
            case eSquare: n = 8; break;
            default: reset(); return false;
        }

        points = new double[n];
        if (!points)
        {
            reset();

            return false;
        }

        bool correctRead = (fread(points, sizeof(double), n, file) == n);

        if( ! correctRead )
        {
            reset();
        }

        return correctRead;
    }
    void draw() const
    {
        if( isValid() )
        {
            switch (type)
            {
                case eCircle: drawCircle(points[0], points[1], points[2]); break;
                case eTriangle: drawPolygon(points, 6); break;
                case eSquare: drawPolygon(points, 8); break;
                case eUnknown: break;
            }
        }
    }

protected:
    void drawCircle(double centerX, double centerY, double radius) const;
    void drawPolygon(const double* points, int size) const;

private:
    void reset()
    {
        type = eUnknown;
        if( points )
        {
            delete [] points;
            points = nullptr;
        }
    }

private:
    double* points;
    FeatureType type;
};

void generateFile(Feature::FeatureType feature)
{
    int size = 0;
    switch(feature)
    {
        case Feature::eCircle:
            size = 3;
            break;

        case Feature::eSquare:
            size = 8;
            break;

        case Feature::eTriangle:
            size = 6;
            break;

        default:
            return;
    }

    double d[size];

    for( int i=0; i<size; ++i )
    {
        d[i] = i + 0.25;
    }

    FILE* file = fopen("features.dat", "w");
    fwrite(&feature, sizeof(feature), 1, file);

    fwrite(d, sizeof (double), size, file);

    fclose(file);
}

int main(int , char* [])
{
    generateFile(Feature::eSquare);
    Feature feature;
    FILE* file = fopen("features.dat", "r");

    feature.read(file);

    if( file )
    {
        fclose(file);
        file = nullptr;
    }

    if (!feature.isValid())
        return 1;
    return 0;
}
