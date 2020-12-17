Function E_N_to_Lat(East, North, a, b, e0, n0, f0, PHI0, LAM0)
'Un-project Transverse Mercator eastings and northings back to latitude.
'Input: - _
 eastings (East) and northings (North) in meters; _
 ellipsoid axis dimensions (a & b) in meters; _
 eastings (e0) and northings (n0) of false origin in meters; _
 central meridian scale factor (f0) and _
 latitude (PHI0) and longitude (LAM0) of false origin in decimal degrees.

'REQUIRES THE "Marc" AND "InitialLat" FUNCTIONS

'Convert angle measures to radians
    Pi = 3.14159265358979
    RadPHI0 = PHI0 * (Pi / 180)
    RadLAM0 = LAM0 * (Pi / 180)

'Compute af0, bf0, e squared (e2), n and Et
    af0 = a * f0
    bf0 = b * f0
    e2 = ((af0 ^ 2) - (bf0 ^ 2)) / (af0 ^ 2)
    n = (af0 - bf0) / (af0 + bf0)
    Et = East - e0

'Compute initial value for latitude (PHI) in radians
    PHId = InitialLat(North, n0, af0, RadPHI0, n, bf0)
    
'Compute nu, rho and eta2 using value for PHId
    nu = af0 / (Sqr(1 - (e2 * ((Sin(PHId)) ^ 2))))
    rho = (nu * (1 - e2)) / (1 - (e2 * (Sin(PHId)) ^ 2))
    eta2 = (nu / rho) - 1
    
'Compute Latitude
    VII = (Tan(PHId)) / (2 * rho * nu)
    VIII = ((Tan(PHId)) / (24 * rho * (nu ^ 3))) * (5 + (3 * ((Tan(PHId)) ^ 2)) + eta2 - (9 * eta2 * ((Tan(PHId)) ^ 2)))
    IX = ((Tan(PHId)) / (720 * rho * (nu ^ 5))) * (61 + (90 * ((Tan(PHId)) ^ 2)) + (45 * ((Tan(PHId)) ^ 4)))
    
    E_N_to_Lat = (180 / Pi) * (PHId - ((Et ^ 2) * VII) + ((Et ^ 4) * VIII) - ((Et ^ 6) * IX))

End Function

Function E_N_to_Long(East, North, a, b, e0, n0, f0, PHI0, LAM0)
'Un-project Transverse Mercator eastings and northings back to longitude.
'Input: - _
 eastings (East) and northings (North) in meters; _
 ellipsoid axis dimensions (a & b) in meters; _
 eastings (e0) and northings (n0) of false origin in meters; _
 central meridian scale factor (f0) and _
 latitude (PHI0) and longitude (LAM0) of false origin in decimal degrees.

'REQUIRES THE "Marc" AND "InitialLat" FUNCTIONS

'Convert angle measures to radians
    Pi = 3.14159265358979
    RadPHI0 = PHI0 * (Pi / 180)
    RadLAM0 = LAM0 * (Pi / 180)

'Compute af0, bf0, e squared (e2), n and Et
    af0 = a * f0
    bf0 = b * f0
    e2 = ((af0 ^ 2) - (bf0 ^ 2)) / (af0 ^ 2)
    n = (af0 - bf0) / (af0 + bf0)
    Et = East - e0

'Compute initial value for latitude (PHI) in radians
    PHId = InitialLat(North, n0, af0, RadPHI0, n, bf0)
    
'Compute nu, rho and eta2 using value for PHId
    nu = af0 / (Sqr(1 - (e2 * ((Sin(PHId)) ^ 2))))
    rho = (nu * (1 - e2)) / (1 - (e2 * (Sin(PHId)) ^ 2))
    eta2 = (nu / rho) - 1
    
'Compute Longitude
    X = ((Cos(PHId)) ^ -1) / nu
    XI = (((Cos(PHId)) ^ -1) / (6 * (nu ^ 3))) * ((nu / rho) + (2 * ((Tan(PHId)) ^ 2)))
    XII = (((Cos(PHId)) ^ -1) / (120 * (nu ^ 5))) * (5 + (28 * ((Tan(PHId)) ^ 2)) + (24 * ((Tan(PHId)) ^ 4)))
    XIIA = (((Cos(PHId)) ^ -1) / (5040 * (nu ^ 7))) * (61 + (662 * ((Tan(PHId)) ^ 2)) + (1320 * ((Tan(PHId)) ^ 4)) + (720 * ((Tan(PHId)) ^ 6)))

    E_N_to_Long = (180 / Pi) * (RadLAM0 + (Et * X) - ((Et ^ 3) * XI) + ((Et ^ 5) * XII) - ((Et ^ 7) * XIIA))

End Function

Function InitialLat(North, n0, afo, PHI0, n, bfo)
'Compute initial value for Latitude (PHI) IN RADIANS.
'Input: - _
 northing of point (North) and northing of false origin (n0) in meters; _
 semi major axis multiplied by central meridian scale factor (af0) in meters; _
 latitude of false origin (PHI0) IN RADIANS; _
 n (computed from a, b and f0) and _
 ellipsoid semi major axis multiplied by central meridian scale factor (bf0) in meters.
 
'REQUIRES THE "Marc" FUNCTION
'THIS FUNCTION IS CALLED BY THE "E_N_to_Lat", "E_N_to_Long" and "E_N_to_C" FUNCTIONS
'THIS FUNCTION IS ALSO USED ON IT'S OWN IN THE  "Projection and Transformation Calculations.xls" SPREADSHEET

'First PHI value (PHI1)
    PHI1 = ((North - n0) / afo) + PHI0
    
'Calculate M
    M = Marc(bfo, n, PHI0, PHI1)
    
'Calculate new PHI value (PHI2)
    PHI2 = ((North - n0 - M) / afo) + PHI1
    
'Iterate to get final value for InitialLat
    Do While Abs(North - n0 - M) > 0.00001
        PHI2 = ((North - n0 - M) / afo) + PHI1
        M = Marc(bfo, n, PHI0, PHI2)
        PHI1 = PHI2
    Loop
    
    InitialLat = PHI2
    
End Function

Function Marc(bf0, n, PHI0, PHI)
'Compute meridional arc.
'Input: - _
 ellipsoid semi major axis multiplied by central meridian scale factor (bf0) in meters; _
 n (computed from a, b and f0); _
 lat of false origin (PHI0) and initial or final latitude of point (PHI) IN RADIANS.

'THIS FUNCTION IS CALLED BY THE - _
 "Lat_Long_to_North" and "InitialLat" FUNCTIONS
'THIS FUNCTION IS ALSO USED ON IT'S OWN IN THE "Projection and Transformation Calculations.xls" SPREADSHEET

    Marc = bf0 * (((1 + n + ((5 / 4) * (n ^ 2)) + ((5 / 4) * (n ^ 3))) * (PHI - PHI0)) _
    - (((3 * n) + (3 * (n ^ 2)) + ((21 / 8) * (n ^ 3))) * (Sin(PHI - PHI0)) * (Cos(PHI + PHI0))) _
    + ((((15 / 8) * (n ^ 2)) + ((15 / 8) * (n ^ 3))) * (Sin(2 * (PHI - PHI0))) * (Cos(2 * (PHI + PHI0)))) _
    - (((35 / 24) * (n ^ 3)) * (Sin(3 * (PHI - PHI0))) * (Cos(3 * (PHI + PHI0)))))

End Function