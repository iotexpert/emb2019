/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/** @file
 *  Enumeration of AWS and other internal error codes
 */
#pragma once

typedef enum
{
    AWS_SUCCESS = 0,

    AWS_ERROR = 1
} AWS_error;


typedef enum
{
    WICED_SUCCESS = 0,

    WICED_ERROR = 1,

    WICED_BADARG = 2,
    WICED_NOT_FOUND = 3
} wiced_result_t;

typedef enum
{
    WICED_FALSE = 0,
    WICED_TRUE  = 1
} wiced_bool_t;
