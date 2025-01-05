class Collision
{
    // ...existing code...
    inline static bool CheckCollisionMask(ECollisionChannel _channel1, ECollisionChannel _channel2)
    {
        return getInstance().GetCollisionMatrix(_channel1, _channel2);
    }

private:
    bool GetCollisionMatrix(ECollisionChannel _channel1, ECollisionChannel _channel2) const
    {
        return CollisionMatrix.at(_channel1).test(static_cast<uint8>(_channel2));
    }
    // ...existing code...
};
